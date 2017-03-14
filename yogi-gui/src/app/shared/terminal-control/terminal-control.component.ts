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
  @Input() lastReceivedPublishMessage: yogi.Message;
  @Input() lastReceivedCachedPublishMessage: yogi.Message;
  @Input() lastReceivedScatterMessage: yogi.ScatterMessage;
  @Input() lastReceivedGatherMessage: yogi.GatherMessage;

  @Output() sendPublishMessage = new EventEmitter();

  private canReceivePublishMsg: boolean = false;
  private canReceiveCachedPublishMsg: boolean = false;
  private canReceiveScatterMsg: boolean = false;
  private canReceiveGatherMsg: boolean = false;
  private canSendPublishMsg: boolean = false;
  private canSendScatterMsg: boolean = false;
  private canSendGatherMsg: boolean = false;

  private lastSendableUpperMsg: yogi.Message | ByteBuffer;
  private lastSendableLowerMsg: yogi.Message | ByteBuffer;

  private dummyUpperValue: any;
  private dummyLowerValue: any;
  private dummyUpperTimestamp: any;
  private dummyLowerTimestamp: any;

  ngOnInit() {
    this.setupGuiVariables();
    this.setupDummyVariablesAndMessages();
  }

  setupGuiVariables() {
    switch (this.terminalType) {
      case yogi.DeafMuteTerminal:
        break;

      case yogi.PublishSubscribeTerminal:
        this.canReceivePublishMsg = this.canReceive;
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.CachedPublishSubscribeTerminal:
        this.canReceivePublishMsg = this.canReceive;
        this.canReceiveCachedPublishMsg = this.canReceive;
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.ScatterGatherTerminal:
        this.canReceiveScatterMsg = this.canReceive;
        this.canReceiveGatherMsg = this.canSend;
        this.canSendScatterMsg = this.canSend;
        this.canSendGatherMsg = this.canReceive;
        break;

      case yogi.ProducerTerminal:
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.ConsumerTerminal:
        this.canReceivePublishMsg = this.canReceive;
        break;

      case yogi.CachedProducerTerminal:
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.CachedConsumerTerminal:
        this.canReceivePublishMsg = this.canReceive;
        this.canReceiveCachedPublishMsg = this.canReceive;
        break;

      case yogi.MasterTerminal:
        this.canReceivePublishMsg = this.canReceive;
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.SlaveTerminal:
        this.canReceivePublishMsg = this.canReceive;
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.CachedMasterTerminal:
        this.canReceivePublishMsg = this.canReceive;
        this.canReceiveCachedPublishMsg = this.canReceive;
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.CachedSlaveTerminal:
        this.canReceivePublishMsg = this.canReceive;
        this.canReceiveCachedPublishMsg = this.canReceive;
        this.canSendPublishMsg = this.canSend;
        break;

      case yogi.ServiceTerminal:
        this.canReceiveScatterMsg = this.canReceive;
        this.canSendGatherMsg = this.canReceive;
        break;

      case yogi.ClientTerminal:
        this.canSendScatterMsg = this.canSend;
        this.canReceiveGatherMsg = this.canSend;
        break;

      default:
        throw new Error(`Unknown terminal type: ${this.terminalType}.`);
    }
  }

  setupDummyVariablesAndMessages() {
    if (!this.signature.isCustom && !this.signature.isReserved) {
      this.lastSendableUpperMsg = yogi.MessageFactory.createMessage(yogi.MessageType.Scatter,
        this.signature);
      this.dummyUpperValue = this.lastSendableUpperMsg.value;

      this.lastSendableLowerMsg = yogi.MessageFactory.createMessage(yogi.MessageType.Publish,
        this.signature);
      this.dummyLowerValue = this.lastSendableLowerMsg.value;

      if (this.signature.upperHalf.hasTimestamp) {
        this.dummyUpperTimestamp = 0;
      }

      if (this.signature.lowerHalf.hasTimestamp) {
        this.dummyLowerTimestamp = 0;
      }
    }
    else {
      this.lastSendableUpperMsg = new ByteBuffer(0);
      this.lastSendableLowerMsg = new ByteBuffer(0);
    }
  }

  onPublishMessageChanged(change: Change) {
    for (let field in change) {
      if (change.hasOwnProperty(field)) {
        this.lastSendableLowerMsg[field] = change[field];
      }
    }
  }

  onSendButtonClicked() {
    console.log(this.lastSendableLowerMsg);
    this.sendPublishMessage.emit(this.lastSendableLowerMsg);
  }
}
