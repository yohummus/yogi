import {
  hexStringToByteBuffer,
  byteBufferToHexString,
  timestampToStringForHtmlInput,
  timestampToTooltipString,
  dateToTimestamp,
} from '../../../helpers/helpers.barrel';

import {
  InputGroupComponent,
} from './input-group.component';

export interface ValuePair {
  first: any;
  second: any;
}

export enum GuiPrimitiveType {
  VOID_PRIMITIVE,
  BUTTON_PRIMITIVE,
  NUMBER_PRIMITIVE,
  TEXT_PRIMITIVE,
  TIMESTAMP_PRIMITIVE
}

export class GuiPrimitiveData {
  private _guiPrimitiveType: GuiPrimitiveType;
  private _value: any;
  private _minValue: number;
  private _maxValue: number;
  private _valueString = '';
  private _isValid = true;
  private _tooltip = '';
  private _makeValueStringFn: (val: any) => string;
  private _makeTooltipFn: (val: any) => string;

  constructor(public component: InputGroupComponent,
              private primitiveType: yogi.PrimitiveType,
              private isReadonly: boolean) {
    switch (primitiveType) {
      case yogi.PrimitiveType.BOOL:
        this._guiPrimitiveType = GuiPrimitiveType.BUTTON_PRIMITIVE;
        this._makeValueStringFn = this._boolOrTriboolToIcon;
        this._makeTooltipFn = this._boolOrTriboolToString;
        this.value = false;
        break;

      case yogi.PrimitiveType.TRIBOOL:
        this._guiPrimitiveType = GuiPrimitiveType.BUTTON_PRIMITIVE;
        this._makeValueStringFn = this._boolOrTriboolToIcon;
        this._makeTooltipFn = this._boolOrTriboolToString;
        this.value = yogi.Tribool.UNDEFINED;
        break;

      case yogi.PrimitiveType.INT8:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = -128;
        this._maxValue = +127;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.UINT8:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = 0;
        this._maxValue = 255;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.INT16:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = -32768;
        this._maxValue = +32767;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.UINT16:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = 0;
        this._maxValue = 65535;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.INT32:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = -2147483648;
        this._maxValue = +2147483647;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.UINT32:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = 0;
        this._maxValue = 4294967295;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.INT64:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = -9223372036854775808;
        this._maxValue = +9223372036854775807;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.UINT64:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._minValue = 0;
        this._maxValue = 18446744073709551615;
        this._makeValueStringFn = (val) => val.toString();
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0;
        break;

      case yogi.PrimitiveType.FLOAT:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._makeValueStringFn = (val) => this._numberToPrecision(val, 7);
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0.0;
        break;

      case yogi.PrimitiveType.DOUBLE:
        this._guiPrimitiveType = GuiPrimitiveType.NUMBER_PRIMITIVE;
        this._makeValueStringFn = (val) => this._numberToPrecision(val, 15);
        this._makeTooltipFn = (val) => `${val}`;
        this.value = 0.0;
        break;

      case yogi.PrimitiveType.STRING:
        this._guiPrimitiveType = GuiPrimitiveType.TEXT_PRIMITIVE;
        this._makeValueStringFn = (val) => val;
        this._makeTooltipFn = (val) => `${val ? val.length : 0} character string`;
        this.value = '';
        break;

      case yogi.PrimitiveType.JSON:
        this._guiPrimitiveType = GuiPrimitiveType.TEXT_PRIMITIVE;
        this._makeValueStringFn = (val) => val;
        this._makeTooltipFn = this._makeJsonTooltip;
        this.value = '{}';
        break;

      case yogi.PrimitiveType.BLOB:
        this._guiPrimitiveType = GuiPrimitiveType.TEXT_PRIMITIVE;
        this._makeValueStringFn = byteBufferToHexString;
        this._makeTooltipFn = (val) => `${val.remaining()} byte BLOB`;
        this.value = new ByteBuffer(0);
        break;

      case yogi.PrimitiveType.TIMESTAMP:
        this._guiPrimitiveType = GuiPrimitiveType.TIMESTAMP_PRIMITIVE;
        this._makeValueStringFn = timestampToStringForHtmlInput;
        this._makeTooltipFn = timestampToTooltipString;
        this.value = 0;
        break;

      default:
        this._guiPrimitiveType = GuiPrimitiveType.VOID_PRIMITIVE;
        this._makeValueStringFn = (val) => null;
        this._makeTooltipFn = (val) => null;
        break;
    }
  }

  get guiPrimitiveType(): GuiPrimitiveType {
    return this._guiPrimitiveType;
  }

  get value(): any {
    return this._value;
  }

  set value(val: any) {
    this.storeValueIfValid(val);
    this._valueString = this._makeValueStringFn(val);
    this._tooltip = this._makeTooltipFn(val);
  }

  get minValue(): number {
    return this._minValue;
  }

  get maxValue(): number {
    return this._maxValue;
  }

  get valueString(): string {
    return this._valueString;
  }

  onBoolButtonClicked() {
    if (this.isReadonly) {
      return;
    }

    switch (this._value) {
      case false:
        this._value = true;
        break;

      case true:
        this._value = false;
        break;

      case yogi.Tribool.TRUE:
        this._value = yogi.Tribool.UNDEFINED;
        break;

      case yogi.Tribool.UNDEFINED:
        this._value = yogi.Tribool.FALSE;
        break;

      case yogi.Tribool.FALSE:
        this._value = yogi.Tribool.TRUE;
        break;

      default:
        throw new Error(`Invalid bool/tribool value: ${this._value}.`);
    }

    this._valueString = this._makeValueStringFn(this._value);
    this._tooltip = this._makeTooltipFn(this._value);

    this.component.onPrimitiveChanged();
  }

  storeValueIfValid(newValue: any) {
    switch (this.primitiveType) {
      case yogi.PrimitiveType.BOOL:
      case yogi.PrimitiveType.TRIBOOL:
        if ([false, true, yogi.Tribool.TRUE, yogi.Tribool.FALSE, yogi.Tribool.UNDEFINED].indexOf(newValue) === -1) {
          this._isValid = false;
        }
        else {
          this._isValid = true;
          this._value = newValue;
        }
        break;

      case yogi.PrimitiveType.INT8:
      case yogi.PrimitiveType.UINT8:
      case yogi.PrimitiveType.INT16:
      case yogi.PrimitiveType.UINT16:
      case yogi.PrimitiveType.INT32:
      case yogi.PrimitiveType.UINT32:
      case yogi.PrimitiveType.INT64:
      case yogi.PrimitiveType.UINT64:
        this._isValid = (Number.isInteger(newValue) || newValue instanceof Long)
                      && newValue >= this._minValue && newValue <= this._maxValue;
        this._value = newValue;
        break;

      case yogi.PrimitiveType.FLOAT:
      case yogi.PrimitiveType.DOUBLE:
        this._isValid = newValue !== null;
        this._value = this.isValid ? newValue : 0;
        break;

      case yogi.PrimitiveType.STRING:
        this._isValid = true;
        this._value = newValue;
        break;

      case yogi.PrimitiveType.JSON:
        try {
          JSON.parse(newValue);
          this._isValid = true;
        }
        catch (err) {
          this._isValid = false;
        }
        this._value = newValue;
        break;

      case yogi.PrimitiveType.BLOB:
        try {
          if (newValue instanceof ByteBuffer) {
            this._value = newValue;
          }
          else {
            this._value = hexStringToByteBuffer(newValue);
          }
          this._isValid = true;
        }
        catch (err) {
          this._value = new ByteBuffer(0);
          this._isValid = false;
        }
        break;

      case yogi.PrimitiveType.TIMESTAMP:
        this._isValid = true;
        this._value = dateToTimestamp(newValue
          ? new Date(newValue)
          : new Date() // now
        );
        break;
    }
  }

  onValueInputChanged(newValue: any) {
    this.storeValueIfValid(newValue);
    this._tooltip = this._makeTooltipFn(this._value);
    this.component.onPrimitiveChanged();
  }

  get isValid(): boolean {
    return this._isValid;
  }

  get tooltip(): string {
    return this._tooltip;
  }

  private _boolOrTriboolToIcon(val: boolean | yogi.Tribool): string {
    switch (val) {
      case false:
      case yogi.Tribool.FALSE:
        return 'fa-toggle-off';

      case true:
      case yogi.Tribool.TRUE:
        return 'fa-toggle-on';

      default:
        return 'fa-question';
    }
  }

  private _boolOrTriboolToString(val: boolean | yogi.Tribool): string {
    switch (val) {
      case false:
      case yogi.Tribool.FALSE:
        return 'false';

      case true:
      case yogi.Tribool.TRUE:
        return 'true';

      default:
        return 'undefined';
    }
  }

  private _numberToPrecision(val: number, precision: number): string {
    let s = val.toPrecision(precision);

    let pos = s.length - 1;
    while (s[pos] === '0') {
      --pos;
    }

    if (pos === s.length - 1) {
      return s;
    }
    else {
      return s.substr(0, pos + 2);
    }
  }

  private _makeJsonTooltip(json: string): string {
    try {
      return JSON.stringify(JSON.parse(json), null, 2);
    }
    catch (err) {
      return `Invalid JSON: ${err}`;
    }
  }
}
