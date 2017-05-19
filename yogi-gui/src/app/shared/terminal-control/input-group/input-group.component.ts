import {
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnDestroy,
  OnInit,
  Output,
  SimpleChanges,
} from '@angular/core';

import {
  ErrorDisplayService
} from '../../../core/core.module';

import {
  hexStringToByteBuffer,
  byteBufferToHexString,
  timestampToStringForHtmlInput,
  timestampToTooltipString,
  dateToTimestamp,
} from '../../../helpers/helpers.barrel';

import {
  ValuePair,
  GuiPrimitiveType,
  GuiPrimitiveData
} from './input-group.helpers';

export interface Change {
  timestamp?: number;
  value: any | ValuePair | any[] | ValuePair[];
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
  @Input() timestamp: number;
  @Input() value: any | ValuePair | any[] | ValuePair[];
  @Input() isListElement: boolean = false;
  @Input() hasSendButton: boolean = false;
  @Input() hasReplyButton: boolean = false;
  @Input() hasGatherButton: boolean = false;
  @Input() isReadonly: boolean = false;

  @Output() changed = new EventEmitter<Change>();
  @Output() sendButtonClicked = new EventEmitter<void>();
  @Output() listModificationButtonClicked = new EventEmitter<boolean>();

  GuiPrimitiveType = GuiPrimitiveType;

  private primitiveValues: GuiPrimitiveData[];
  private timestampValue: number = 0;
  private timestampString: string;
  private timestampTitle: string;
  private viewValue: any | ValuePair | any[] | ValuePair[];

  constructor(private errorDisplayService: ErrorDisplayService) {
  }

  ngOnInit() {
    this.setupPrimitiveValues();

    if (this.viewValue !== null && typeof this.viewValue !== 'undefined') {
      this.updatePrimitiveValues();
    }

    if (this.timestamp !== null && typeof this.timestamp !== 'undefined') {
      this.updateTimestamp(this.timestamp);
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
      this.viewValue = this.value;

      if (this.primitiveValues) {
        this.updatePrimitiveValues();
      }
    }

    if ('timestamp' in changes) {
      this.updateTimestamp(this.timestamp);
    }
  }

  get isOfficial(): boolean {
    return !!this.signatureHalf;
  }

  get isList(): boolean {
    return this.isOfficial && this.signatureHalf.isList && !this.isListElement;
  }

  get hasTimestamp(): boolean {
    return this.isOfficial && this.signatureHalf.hasTimestamp && !this.isListElement;
  }

  setupPrimitiveValues() {
    if (this.isOfficial) {
      this.primitiveValues = [new GuiPrimitiveData(this,
        this.signatureHalf.primitiveTypes.first, this.isReadonly)];

      if (this.signatureHalf.isPair) {
        this.primitiveValues.unshift(new GuiPrimitiveData(this,
          this.signatureHalf.primitiveTypes.second, this.isReadonly));
      }
    }
    else {
      this.primitiveValues = [new GuiPrimitiveData(this,
        yogi.PrimitiveType.BLOB, this.isReadonly)];
    }
  }

  updatePrimitiveValues() {
    if (this.viewValue === null || typeof this.viewValue === 'undefined' || this.isList) {
      return;
    }

    if (this.isOfficial && this.signatureHalf.isPair) {
      this.primitiveValues[0].value = this.viewValue.second;
      this.primitiveValues[1].value = this.viewValue.first;
    }
    else {
      this.primitiveValues[0].value = this.viewValue;
    }
  }

  updateTimestamp(timestamp?: number) {
    if (!timestamp) {
      timestamp = 0;
    }

    this.timestampValue = timestamp;
    this.timestampString = timestampToStringForHtmlInput(timestamp);
    this.timestampTitle = timestampToTooltipString(timestamp);
  }

  onPrimitiveChanged() {
    for (let primitiveValue of this.primitiveValues) {
      if (!primitiveValue.isValid) {
        return;
      }
    }

    this.emitChange();
  }

  emitChange() {
    let value;
    if (this.isOfficial && this.signatureHalf.isPair) {
      value = {
        first: this.primitiveValues[1].value,
        second: this.primitiveValues[0].value
      };
    }
    else {
      value = this.primitiveValues[0].value;
    }

    if (this.hasTimestamp) {
      let timestamp = this.timestampValue;
      this.changed.emit({ timestamp, value });
    }
    else {
      this.changed.emit({ value });
    }
  }

  onTimestampChanged(newValue: string) {
    let timestamp = dateToTimestamp(new Date(newValue));
    this.updateTimestamp(timestamp);

    this.emitChange();
  }

  onListElementChanged(idx: number, change: Change) {
    console.log(idx, change.value, this.viewValue);
    // this.viewValue[idx] = change.value; // WTF? TODO
  }

  onSendButtonClickedOrEnterPressed() {
    for (let primitiveValue of this.primitiveValues) {
      if (!primitiveValue.isValid) {
        this.errorDisplayService.show('Cannot send invalid value.',
          `The value "${primitiveValue.valueString}" you are trying to send is not valid for the `
          + 'Terminal.');
        return;
      }
    }

    this.sendButtonClicked.emit();
  }

  makeListValue(): any {
    if (this.isOfficial && this.signatureHalf.isPair) {
      return {
        first: this.primitiveValues[1].value,
        second: this.primitiveValues[0].value
      };
    }
    else {
      return this.primitiveValues[0].value;
    }
  }

  onListAppendButtonClicked() {
    let value = this.makeListValue();
    this.viewValue.push(value);
  }

  onListModificationButtonClicked(idx: number, insert: boolean) {
    console.log(idx, insert, this.viewValue);
    if (insert) {
      let value = this.makeListValue();
      this.viewValue.splice(idx, 0, value);
    }
    else /* remove */ {
      this.viewValue.splice(idx, 1);
    }
  }

  onListInsertButtonClicked() {
    this.listModificationButtonClicked.emit(true);
  }

  onListRemoveButtonClicked() {
    this.listModificationButtonClicked.emit(false);
  }
}
