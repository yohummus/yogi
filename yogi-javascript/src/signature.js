(function () {
    let PrimitiveType = {
        VOID      : 0,
        BOOL      : 1,
        TRIBOOL   : 2,
        INT8      : 3,
        UINT8     : 4,
        INT16     : 5,
        UINT16    : 6,
        INT32     : 7,
        UINT32    : 8,
        INT64     : 9,
        UINT64    : 10,
        FLOAT     : 11,
        DOUBLE    : 12,
        STRING    : 13,
        JSON      : 14,
        BLOB      : 15,
        TIMESTAMP : 16,
        INVALID   : 17
    };

    let Tribool = {
        FALSE     : 0,
        TRUE      : 1,
        UNDEFINED : 2
    };

    const PRIMITIVE_TYPE_TO_STRING = [
        'void',
        'bool',
        'tribool',
        'int8',
        'uint8',
        'int16',
        'uint16',
        'int32',
        'uint32',
        'int64',
        'uint64',
        'float',
        'double',
        'string',
        'JSON',
        'BLOB',
        'Timestamp',
        'INVALID'
    ];

    const PRIMITIVE_TYPE_TO_PROTO_TYPE = [
        'INVALID',
        'bool',
        'Tribool',
        'sint32',
        'uint32',
        'sint32',
        'uint32',
        'sint32',
        'uint32',
        'sint64',
        'uint64',
        'float',
        'double',
        'string',
        'string',
        'bytes',
        'uint64',
        'INVALID'
    ];

    class OfficialSignatureHalf {
        constructor(rawHalf) {
            this._rawHalf = rawHalf;
        }

        get raw() {
            return this._rawHalf;
        }

        get isList() {
            return !!((this._rawHalf >> 10) & 1);
        }

        get isPair() {
            return this._bitsToPrimitiveType(5) !== PrimitiveType.VOID;
        }

        get hasTimestamp() {
            return !!((this._rawHalf >> 11) & 1);
        }

        get hasData() {
            return this.primitiveTypes.first !== PrimitiveType.VOID;
        }

        get primitiveTypes() {
            return {
                first  : this._bitsToPrimitiveType(0),
                second : this._bitsToPrimitiveType(5)
            };
        }

        get primitiveTypeNames() {
            return {
                first  : PRIMITIVE_TYPE_TO_STRING[this._bitsToPrimitiveType(0)],
                second : PRIMITIVE_TYPE_TO_STRING[this._bitsToPrimitiveType(5)]
            };
        }

        get primitiveProtoTypeNames() {
            return {
                first  : PRIMITIVE_TYPE_TO_PROTO_TYPE[this._bitsToPrimitiveType(0)],
                second : PRIMITIVE_TYPE_TO_PROTO_TYPE[this._bitsToPrimitiveType(5)]
            };
        }

        get isReserved() {
            let primitives = this.primitiveTypes;
            return (primitives.first === PrimitiveType.INVALID)
                || (primitives.second === PrimitiveType.INVALID)
                || (primitives.first === PrimitiveType.VOID && primitives.second !== PrimitiveType.VOID)
                || (this.isList && primitives.first === PrimitiveType.VOID);
        }

        toString() {
            if (this.isReserved) {
                return 'reserved';
            }

            let str = '';
            if (this.hasTimestamp) {
                str += this.hasData ? 'timestamped ' : 'timestamp';
            }

            if (this.isList) {
                str += 'list of ';
            }

            let names = this.primitiveTypeNames;
            if (this.isPair) {
                str += `pair(${names.second}, ${names.first})`;
            }
            else {
                str += names.first;
            }

            return str;
        }

        _bitsToPrimitiveType(offset) {
            let bits = (this._rawHalf >> offset) & 0b11111;
            if (bits >= PrimitiveType.INVALID) {
                return PrimitiveType.INVALID;
            }

            return bits;
        }
    }

    class Signature {
        constructor(raw) {
            this._raw = raw;

            if (!this.isCustom) {
                this._lowerHalf = new OfficialSignatureHalf(raw);
                this._upperHalf = new OfficialSignatureHalf(raw >> 12);
            }
        }

        get raw() {
            return this._raw;
        }

        get isCustom() {
            return this._raw > 0x00FFFFFF;
        }

        get lowerHalf() {
            return this._lowerHalf;
        }

        get upperHalf() {
            return this._upperHalf;
        }

        get isReserved() {
            if (this.isCustom) {
                return false;
            }

            return this._lowerHalf.isReserved || this._upperHalf.isReserved;
        }

        get bytes() {
            return [
                (this._raw >>  0) & 0xFF,
                (this._raw >>  8) & 0xFF,
                (this._raw >> 16) & 0xFF,
                (this._raw >> 24) & 0xFF
            ];
        }

        get bits() {
            let arr = [];
            for (let i = 0; i < 32; ++i) {
                arr.push((this._raw >> i) & 1);
            }

            return arr;
        }

        get hex() {
            let str = this._raw.toString(16);
            let prefix = '00000000'.substr(0, 8 - str.length);
            return prefix + str;
        }

        toString() {
            let hint;
            if (this.isCustom) {
                hint = 'custom';
            }
            else if (this.isReserved) {
                hint = 'reserved';
            }
            else {
                hint = 'official';
            }

            return `Signature ${this.hex} (${hint})`;
        }
    }

    class ProtoFile {
        constructor(signature) {
            this._signature = signature instanceof Signature ? signature : new Signature(signature);

            if (this._signature.isReserved || this._signature.isCustom) {
                throw new Error('Cannot create ProtoFile for reserved or custom signatures');
            }

            this._package = `yogi_${this._signature.hex}`;

            this._content = [
                'syntax = "proto3";',
                '',
                `package ${this._package};`,
                '',
                ''
            ].join('\r\n');

            let usesTribool = this._signature.lowerHalf.primitiveTypes.first  === PrimitiveType.TRIBOOL
                           || this._signature.lowerHalf.primitiveTypes.second === PrimitiveType.TRIBOOL
                           || this._signature.upperHalf.primitiveTypes.first  === PrimitiveType.TRIBOOL
                           || this._signature.upperHalf.primitiveTypes.second === PrimitiveType.TRIBOOL;
            if (usesTribool) {
                this._content += [
                    'enum Tribool {',
                    '    FALSE = 0;',
                    '    TRUE = 1;',
                    '    UNDEFINED = 2;',
                    '}',
                    ''
                ].join('\r\n');
            }

            this._content += this._makeMessageDefinition('ScatterMessage', this._signature.upperHalf);
            this._content += this._makeMessageDefinition('GatherMessage',  this._signature.lowerHalf);
            this._content += this._makeMessageDefinition('PublishMessage', this._signature.lowerHalf);
        }

        get signature() {
            return this._signature;
        }

        get content() {
            return this._content;
        }

        get package() {
            return this._package;
        }

        get filenameBase() {
            return this._package;
        }

        get filename() {
            return this.filenameBase + '.proto';
        }

        _makeMessageDefinition(messageName, signatureHalf) {
            let lines = [];

            lines.push(`message ${messageName} {`);

            if (signatureHalf.isPair) {
                lines.push('    message Pair {')
                lines.push(`        ${signatureHalf.primitiveProtoTypeNames.first} first = 1;`);
                lines.push(`        ${signatureHalf.primitiveProtoTypeNames.second} second = 2;`);
                lines.push('    }');
                lines.push('');
            }

            if (signatureHalf.hasTimestamp) {
                lines.push('    uint64 timestamp = 1;')
            }

            var prefix = signatureHalf.isList ? '    repeated ' : '    ';
            if (signatureHalf.isPair) {
                lines.push(`${prefix}Pair value = 2;`);
            }
            else if (signatureHalf.primitiveTypes.first !== PrimitiveType.VOID) {
                var suffix = '';
                if (signatureHalf.isList && !signatureHalf.isPair && ['string', 'bytes'].indexOf(signatureHalf.primitiveProtoTypeNames.first) === -1) {
                    suffix = ' [packed=true]';
                }

                lines.push(`${prefix}${signatureHalf.primitiveProtoTypeNames.first} value = 2${suffix};`);
            }

            lines.push('}');
            lines.push('');

            return lines.join('\r\n') + '\r\n';
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.PrimitiveType = PrimitiveType;
    window.yogi.Tribool = Tribool;
    window.yogi.OfficialSignatureHalf = OfficialSignatureHalf;
    window.yogi.Signature             = Signature;
    window.yogi.ProtoFile             = ProtoFile;
})();
