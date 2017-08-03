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

export class ReceivedRegularMessage {
  cached: boolean;
  msg: yogi.Message | Uint8Array;
};

export class ScatterGatherReply {
  msg: yogi.Message | ByteBuffer;
  ignore: boolean;
};

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
  @Input() lastReceivedRegularMessage: ReceivedRegularMessage;
  @Input() lastReceivedScatterMessage: yogi.ScatterMessage;
  @Input() lastReceivedGatherMessage: yogi.GatherMessage;

  @Output() sendRegularMessage = new EventEmitter<yogi.Message | ByteBuffer>();
  @Output() sendScatterMessage = new EventEmitter<yogi.Message | ByteBuffer>();
  @Output() scatterGatherReplyChanged = new EventEmitter<ScatterGatherReply>();

  private canReceiveRegularMsg: boolean = false;
  private canReceiveScatterMsg: boolean = false;
  private canReceiveGatherMsg: boolean = false;
  private canSendRegularMsg: boolean = false;
  private canSendScatterMsg: boolean = false;
  private canSendGatherMsg: boolean = false;

  private sendSignatureHalf: yogi.OfficialSignatureHalf;
  private recvSignatureHalf: yogi.OfficialSignatureHalf;

  private lastSendableRegularMsg: yogi.Message | ByteBuffer;
  private lastSendableScatterMsg: yogi.Message | ByteBuffer;
  private lastSendableGatherMsg: yogi.Message | ByteBuffer;

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
        this.canSendRegularMsg = this.canSend;
        this.sendSignatureHalf = this.signature.upperHalf;
        this.recvSignatureHalf = this.signature.lowerHalf;
        break;

      case yogi.CachedSlaveTerminal:
        this.canReceiveRegularMsg = this.canReceive;
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

    if (!this.signature.representsProtoMessage) {
      this.sendSignatureHalf = null;
      this.recvSignatureHalf = null;
    }
  }

  setupDummyVariablesAndMessages() {
    if (typeof this.sendSignatureHalf === 'undefined') {
      return;
    }

    if (this.signature.representsProtoMessage) {
      let msgType = this.sendSignatureHalf.raw === this.signature.upperHalf.raw
                  ? yogi.MessageType.Master
                  : yogi.MessageType.Slave;
      this.lastSendableRegularMsg = yogi.MessageFactory.createMessage(msgType, this.signature);
      this.lastSendableScatterMsg = yogi.MessageFactory.createMessage(yogi.MessageType.Scatter, this.signature);
      this.lastSendableGatherMsg = yogi.MessageFactory.createMessage(yogi.MessageType.Gather, this.signature);
      this.dummySendableValue = this.lastSendableRegularMsg.value;

      if (this.sendSignatureHalf.hasTimestamp) {
        this.dummySendableTimestamp = 0;
      }
    }
    else {
      this.lastSendableRegularMsg = new ByteBuffer(0);
      this.lastSendableScatterMsg = new ByteBuffer(0);
      this.lastSendableGatherMsg = new ByteBuffer(0);
      this.dummySendableValue = new ByteBuffer(0);
    }
  }

  onSendableRegularMsgChanged(change: Change) {
    this.lastSendableRegularMsg = this.changeMessage(this.lastSendableRegularMsg, change);
  }

  onSendableGatherMsgChanged(change: Change) {
    this.lastSendableGatherMsg = this.changeMessage(this.lastSendableGatherMsg, change);

    this.scatterGatherReplyChanged.emit({
      msg: this.lastSendableGatherMsg,
      ignore: change.ignoreScatteredMsg
    });
  }

  onSendableScatterMsgChanged(change: Change) {
    this.lastSendableScatterMsg = this.changeMessage(this.lastSendableScatterMsg, change);
  }

  changeMessage(msg: yogi.Message | ByteBuffer, change: Change): yogi.Message | ByteBuffer {
    if (change.valid) {
      if (msg instanceof yogi.Message) {
        msg['value'] = change.value;
        if (change.hasOwnProperty('timestamp')) {
          msg['timestamp'] = change.timestamp;
        }
      }
      else {
        msg = change.value;
      }
    }

    return msg;
  }

  getMessageValue(msg: yogi.Message | ByteBuffer) {
    if (msg instanceof yogi.Message) {
      return msg.value;
    }
    else {
      return msg;
    }
  }

  onSendButtonClicked() {
    if (this.canSendScatterMsg) {
      this.sendScatterMessage.emit(this.lastSendableScatterMsg);
    }
    else {
      this.sendRegularMessage.emit(this.lastSendableRegularMsg);
    }
  }
}
