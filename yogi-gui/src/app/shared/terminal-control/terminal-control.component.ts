import {
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output
} from '@angular/core';

import {
  Change,
} from './input-group/input-group.component';

@Component({
  selector: 'ce-terminal-control',
  templateUrl: 'terminal-control.component.html',
  styleUrls: ['terminal-control.component.css']
})
export class TerminalControlComponent implements OnInit {
  @Input() terminalType: any;
  @Input() signature: yogi.Signature;
  @Input() canSend: boolean = true;
  @Input() canReceive: boolean = true;
  @Input() bound: boolean;
  @Input() subscribed: boolean;
  @Input() lastReceivedRegularMessage: yogi.Message;
  @Input() lastReceivedCachedRegularMessage: yogi.Message;
  @Input() lastReceivedScatterMessage: yogi.ScatterMessage;
  @Input() lastReceivedGatherMessage: yogi.GatherMessage;

  @Output() sendRegularMessage = new EventEmitter<yogi.Message | ByteBuffer>();

  private canReceiveRegularMsg: boolean = false;
  private canReceiveCachedRegularMsg: boolean = false;
  private canReceiveScatterMsg: boolean = false;
  private canReceiveGatherMsg: boolean = false;
  private canSendRegularMsg: boolean = false;
  private canSendScatterMsg: boolean = false;
  private canSendGatherMsg: boolean = false;

  private sendSignatureHalf: yogi.OfficialSignatureHalf;
  private recvSignatureHalf: yogi.OfficialSignatureHalf;

  private lastSendableMsg: yogi.Message | ByteBuffer;
  private dummySendableValue: any;
  private dummySendableTimestamp: any;

  ngOnInit() {
    this.setupGuiVariables();
    this.setupDummyVariablesAndMessages();
  }

  setupGuiVariables() {
    switch (this.terminalType) {
      case yogi.DeafMuteTerminal:
        break;

      case yogi.PublishSubscribeTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.lowerHalf;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.CachedPublishSubscribeTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.canReceiveCachedRegularMsg = this.canReceive;
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.lowerHalf;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.ScatterGatherTerminal:
        this.canReceiveScatterMsg = this.canReceive;
        this.canReceiveGatherMsg = this.canSend;
        this.canSendScatterMsg = this.canSend;
        this.canSendGatherMsg = this.canReceive;
        this.sendSignatureHalf = this.signature.upperHalf;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.ProducerTerminal:
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.ConsumerTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.CachedProducerTerminal:
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.CachedConsumerTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.canReceiveCachedRegularMsg = this.canReceive;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.MasterTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.upperHalf;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.SlaveTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.lowerHalf;
        this.recvSignatureHalf = this.signature.upperHalf;
        break;

      case yogi.CachedMasterTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.canReceiveCachedRegularMsg = this.canReceive;
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.upperHalf;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.CachedSlaveTerminal:
        this.canReceiveRegularMsg = this.canReceive;
        this.canReceiveCachedRegularMsg = this.canReceive;
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.lowerHalf;
        this.recvSignatureHalf = this.signature.upperHalf;
        break;

      case yogi.ServiceTerminal:
        this.canReceiveScatterMsg = this.canReceive;
        this.canSendGatherMsg = this.canReceive;
        this.sendSignatureHalf = this.signature.lowerHalf;
        this.recvSignatureHalf = this.signature.upperHalf;
        break;

      case yogi.ClientTerminal:
        this.canSendScatterMsg = this.canSend;
        this.canReceiveGatherMsg = this.canSend;
        this.sendSignatureHalf = this.signature.upperHalf;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      default:
        throw new Error(`Unknown terminal type: ${this.terminalType}.`);
    }
  }

  setupDummyVariablesAndMessages() {
    if (!this.sendSignatureHalf) {
      return;
    }

    if (!this.signature.isCustom && !this.signature.isReserved) {
      let msgType = this.sendSignatureHalf.raw === this.signature.upperHalf.raw
                  ? yogi.MessageType.Master
                  : yogi.MessageType.Slave;
      this.lastSendableMsg = yogi.MessageFactory.createMessage(msgType, this.signature);
      this.dummySendableValue = this.lastSendableMsg.value;

      if (this.sendSignatureHalf.hasTimestamp) {
        this.dummySendableTimestamp = 0;
      }
    }
    else {
      this.lastSendableMsg = new ByteBuffer(0);
    }
  }

  onRegularMessageChanged(change: Change) {
    for (let field in change) {
      if (change.hasOwnProperty(field)) {
        this.lastSendableMsg[field] = change[field];
      }
    }
  }

  onSendButtonClicked() {
    if (this.lastSendableMsg) {
      console.log((this.lastSendableMsg as any).value, (this.lastSendableMsg as any).timestamp);
    }
    
    this.sendRegularMessage.emit(this.lastSendableMsg);
  }
}
