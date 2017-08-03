import {
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnDestroy,
  OnInit,
  Output,
  QueryList,
  SimpleChanges,
  ViewChildren,
} from '@angular/core';

import {
  ErrorDisplayService,
} from '../../../core/core.module';

import {
  hexStringToByteBuffer,
  byteBufferToHexString,
  timestampToStringForHtmlInput,
  timestampToTooltipString,
  dateToTimestamp,
  stringFromHtmlInputToTimestamp
} from '../../../helpers/helpers.barrel';

import {
  ValuePair,
  GuiPrimitiveType,
  GuiPrimitiveData,
} from './input-group.helpers';

export interface Change {
  timestamp?: Long;
  value: any | ValuePair | any[] | ValuePair[];
  valid: boolean;
  ignoreScatteredMsg?: boolean;
}

@Component({
  selector: 'ce-input-group',
  templateUrl: 'input-group.component.html',
  styleUrls: ['input-group.component.css']
})
export class InputGroupComponent implements OnInit, OnChanges, OnDestroy {
  @Input() label: string;
  @Input() enabled: boolean = true;
  @Input() signatureHalf: yogi.OfficialSignatureHalf;
  @Input() timestamp: Long;
  @Input() value: any | ValuePair | any[] | ValuePair[] | ByteBuffer;
  @Input() cached: boolean = false;
  @Input() isListElement: boolean = false;
  @Input() hasSendButton: boolean = false;
  @Input() hasReplyButton: boolean = false;
  @Input() hasGatherButton: boolean = false;
  @Input() isReadonly: boolean = false;

  @Output() changed = new EventEmitter<Change>();
  @Output() sendButtonClicked = new EventEmitter<void>();
  @Output() listModificationButtonClicked = new EventEmitter<boolean>();

  @ViewChildren(InputGroupComponent) listViewChildren: QueryList<InputGroupComponent>;

  GuiPrimitiveType = GuiPrimitiveType;

  private primitiveValues: GuiPrimitiveData[];
  private timestampValue: Long;
  private timestampString: string;
  private timestampTitle: string;
  private listValues: any[];
  private listValuesValid: boolean = true;
  private ignoreScatterMsg: boolean = true;

  constructor(private errorDisplayService: ErrorDisplayService) {
  }

  //===== INITIALISATION AND CHANGE DETECTION =====
  ngOnInit() {
    this.setupPrimitiveValues();

    if (this.isList) {
      this.setupListValues();
    }

    if (this.hasTimestamp) {
      this.setupTimestamp();
    }
  }

  ngOnDestroy() {
    for (let primitive of this.primitiveValues) {
      // remove circular reference
      primitive.component = null;
    }
  }

  ngOnChanges(changes: SimpleChanges) {
    if ('value' in changes) {
      if (this.isList) {
        if (this.listValues) {
          this.updateListValues();
        }
      }
      else {
        if (this.primitiveValues) {
          this.updatePrimitiveValues();
        }
      }
    }

    if ('timestamp' in changes) {
      this.updateTimestamp(this.timestamp);
    }
  }

  get representsProtoMessage(): boolean {
    return !!this.signatureHalf;
  }

  get isList(): boolean {
    return this.representsProtoMessage && this.signatureHalf.isList && !this.isListElement;
  }

  get hasTimestamp(): boolean {
    return this.representsProtoMessage && this.signatureHalf.hasTimestamp && !this.isListElement;
  }


  //===== VALUE LISTS =====
  setupListValues() {
    this.listValues = [];
  }

  updateListValues() {
    this.listValues = this.value;
  }

  makeListValue(): any {
    if (this.representsProtoMessage && this.signatureHalf.isPair) {
      return {
        first: this.primitiveValues[1].value,
        second: this.primitiveValues[0].value
      };
    }
    else {
      return this.primitiveValues[0].value;
    }
  }

  onListModificationButtonClicked(idx: number, insert: boolean) {
    let values = this.getListValues();

    if (insert) {
      let prevValue = values.length ? values[idx] : this.makeListValue();
      values.splice(idx, 0, prevValue);
    }
    else /* remove */ {
      values.splice(idx, 1);
    }

    this.listValues = values;
    this.emitChange(this.listValues);
  }

  onListAppendButtonClicked() {
    this.onListModificationButtonClicked(this.listValues.length, true);
  }

  onListInsertButtonClicked() {
    this.listModificationButtonClicked.emit(true);
  }

  onListRemoveButtonClicked() {
    this.listModificationButtonClicked.emit(false);
  }

  onListValueChanged(idx: number, change: Change) {
    this.listValuesValid = this.areListValuesValid();
    this.emitChange(this.getListValues());
  }

  getListValues(): any[] {
    let values: any[];
    if (this.representsProtoMessage && this.signatureHalf.isPair) {
      return this.listViewChildren.map((child) => {
        return {
          first: child.primitiveValues[1].value,
          second: child.primitiveValues[0].value
        };
      });
    }
    else {
      return this.listViewChildren.map((child) => child.primitiveValues[0].value);
    }
  }

  areListValuesValid(): boolean {
    let foundInvalidValue = this.listViewChildren.find((child) => {
      for (let pv of child.primitiveValues) {
        if (!pv.isValid) {
          return true;
        }
      }

      return false;
    });

    return !foundInvalidValue;
  }

  trackListByIndex(idx: number, val: any): number {
    return idx;
  }


  //===== PRIMITIVES =====
  setupPrimitiveValues() {
    if (this.representsProtoMessage) {
      this.primitiveValues = [new GuiPrimitiveData(this, this.signatureHalf.primitiveTypes.first, this.isReadonly)];

      if (this.signatureHalf.isPair) {
        this.primitiveValues.unshift(new GuiPrimitiveData(this, this.signatureHalf.primitiveTypes.second, this.isReadonly));
      }
    }
    else {
      this.primitiveValues = [new GuiPrimitiveData(this, yogi.PrimitiveType.BLOB, this.isReadonly)];
    }

    if (this.value !== null && !this.isList) {
      this.updatePrimitiveValues();
    }
  }

  updatePrimitiveValues() {
    if (this.representsProtoMessage && this.signatureHalf.isPair) {
      this.primitiveValues[0].value = this.value.second;
      this.primitiveValues[1].value = this.value.first;
    }
    else {
      this.primitiveValues[0].value = this.value;
    }
  }

  onPrimitiveChanged() {
    this.emitChange();
  }


  //===== TIMESTAMPS =====
  setupTimestamp() {
    if (this.timestamp !== null && typeof this.timestamp !== 'undefined') {
      this.updateTimestamp(this.timestamp);
    }
  }

  updateTimestamp(timestamp?: Long) {
    if (!timestamp) {
      timestamp = new Long(0);
    }

    this.timestampValue = timestamp;
    this.timestampString = timestampToStringForHtmlInput(timestamp);
    this.timestampTitle = timestampToTooltipString(timestamp);
  }

  onTimestampChanged(newValue: string) {
    let timestamp = stringFromHtmlInputToTimestamp(newValue);
    this.timestampValue = timestamp;
    this.timestampTitle = timestampToTooltipString(timestamp);

    this.emitChange();
  }


  //===== Scatter-Gather-specific =====
  onReplyButtonClicked() {
    this.ignoreScatterMsg = !this.ignoreScatterMsg;
    this.emitChange(this.getListValues());
  }


  //===== OUTPUTS =====
  areValuesValid(): boolean {
    if (this.isList) {
      return this.listValuesValid;
    }
    else {
      return !this.primitiveValues.find((pv) => pv.isValid === false);
    }
  }

  emitChange(listValues?: any[]) {
    let value;
    if (this.isList) {
      value = listValues ? listValues : [];
    }
    else if (this.representsProtoMessage && this.signatureHalf.isPair) {
      value = {
        first: this.primitiveValues[1].value,
        second: this.primitiveValues[0].value
      };
    }
    else {
      value = this.primitiveValues[0].value;
    }

    let change: Change = {
      valid: this.areValuesValid(),
      value: value
    };

    if (this.hasTimestamp) {
      change.timestamp = this.timestampValue;
    }

    if (this.hasReplyButton) {
      change.ignoreScatteredMsg = this.ignoreScatterMsg;
    }

    this.changed.emit(change);
  }

  onSendButtonClickedOrEnterPressed() {
    if (this.areValuesValid()) {
      this.sendButtonClicked.emit();
    }
    else {
      this.errorDisplayService.show('Cannot send message with invalid values.',
        'The message you are trying to send contains values that are not valid for the Terminal.');
    }
  }
}
