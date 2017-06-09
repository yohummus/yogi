import {
  Component,
} from '@angular/core';

import {
  ErrorDisplayService,
} from '../../core/core.module';

@Component({
  selector: 'ce-signatures-page',
  templateUrl: 'signatures.component.html',
  styleUrls: ['signatures.component.css']
})
export class SignaturesPageComponent {
  public static pageTitle = 'Signatures';

  private primitiveTypeNames: string[] = [];
  private rawSignatureValidPattern = '^[0-9a-fA-F]{1,8}$';
  private signature: yogi.Signature | void;
  private _rawSignatureString = '00000000';
  private downloading: string | void = null;
  private compiler = new yogi.ProtoCompiler();

  constructor(private errorDisplayService: ErrorDisplayService) {
    for (let name in yogi.PrimitiveType) {
      if (name !== 'INVALID') {
        this.primitiveTypeNames.push(name);
      }
    }

    this.rawSignatureString = this._rawSignatureString;
  }

  get rawSignatureString() {
    return this._rawSignatureString;
  }

  set rawSignatureString(str) {
    if (str.match(this.rawSignatureValidPattern)) {
      this._rawSignatureString = str.toUpperCase();
      this.signature = new yogi.Signature(parseInt(str, 16));
    }
    else {
      this._rawSignatureString = str;
      this.signature = null;
    }
  }

  get signatureType() {
    if (this.signature) {
      if (this.signature.isCustom) {
        return 'custom';
      }
      else {
        return this.signature.isReserved ? 'reserved' : 'official';
      }
    }
    else {
      return 'invalid';
    }
  }

  get isOfficial() {
    return !!this.signature && !this.signature.isCustom && !this.signature.isReserved;
  }

  timestampEnabled(upperHalf: boolean) {
    let sig = (this.signature as yogi.Signature);
    if (upperHalf) {
      return sig.upperHalf.hasTimestamp;
    }
    else {
      return sig.lowerHalf.hasTimestamp;
    }
  }

  listEnabled(upperHalf: boolean) {
    let sig = (this.signature as yogi.Signature);
    if (upperHalf) {
      return sig.upperHalf.isList;
    }
    else {
      return sig.lowerHalf.isList;
    }
  }

  selectedPrimitiveTypeName(upperHalf: boolean, firstPrimitive: boolean) {
    let half = upperHalf
             ? (this.signature as yogi.Signature).upperHalf
             : (this.signature as yogi.Signature).lowerHalf;

    let name = firstPrimitive
             ? half.primitiveTypeNames.first
             : half.primitiveTypeNames.second;

    return name;
  }

  primitiveTypeSelectionEnabled(upperHalf: boolean, firstPrimitive: boolean) {
    if (firstPrimitive) {
      return true;
    }

    let sig = this.signature as yogi.Signature;
    if (upperHalf) {
      return sig.upperHalf.primitiveTypes.first !== yogi.PrimitiveType.VOID;
    }
    else {
      return sig.lowerHalf.primitiveTypes.first !== yogi.PrimitiveType.VOID;
    }
  }

  onTimestampButtonClicked(upperHalf: boolean) {
    let raw = (this.signature as yogi.Signature).raw;
    let shiftAmount = upperHalf ? 23 : 11;
    raw ^= 1 << shiftAmount;

    this.rawSignatureString = new yogi.Signature(raw).hex;
  }

  onListButtonClicked(upperHalf: boolean) {
    let raw = (this.signature as yogi.Signature).raw;
    let shiftAmount = upperHalf ? 22 : 10;
    raw ^= 1 << shiftAmount;

    this.rawSignatureString = new yogi.Signature(raw).hex;
  }

  onDownloadButtonClicked(language: string) {
    if (!this.isOfficial) {
      return;
    }

    let file = new yogi.ProtoFile(this.signature as yogi.Signature);

    let promise: Promise<any>;
    switch (language) {
      case 'Proto':
        promise = new Promise<void>((resolve) => {
          yogi.downloadTextFile(file.filename, file.content);
          resolve();
        });
        break;

      case 'C++':
        promise = this.compiler.compileToCppAndDownload(file);
        break;

      case 'Python':
        promise = this.compiler.compileToPythonAndDownload(file);
        break;

      default:
        throw new Error(`Unknown language "${language}"`);
    }

    this.downloading = language;
    promise
    .then(() => {
      this.downloading = null;
    })
    .catch((err) => {
      this.downloading = null;
      this.errorDisplayService.show(`Generating ${language} files failed`, err);
    });
  }

  onPrimitiveTypeSelected(upperHalf: boolean, firstPrimitive: boolean, typeName: string) {
    let shiftAmount = 0;
    if (upperHalf) {
      shiftAmount += 12;
    }
    if (!firstPrimitive) {
      shiftAmount += 5;
    }

    let type = yogi.PrimitiveType[typeName];
    let raw = (this.signature as yogi.Signature).raw;

    raw &= ~(0x1F << shiftAmount);
    raw |= (type << shiftAmount);

    this.signature = new yogi.Signature(raw);
    if (!this.primitiveTypeSelectionEnabled(upperHalf, false)) {
      raw &= ~(0x1F << (upperHalf ? 17 : 5));
      this.signature = new yogi.Signature(raw);
    }

    this.rawSignatureString = this.signature.hex;
  }
}
