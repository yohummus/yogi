declare module yogi {
    const hubProtocol: string;
    const hubHost: string;
    const hubPort: number;
    const hubUri: string;

    export function downloadTextFile(filename: string, content: string): void;

    export enum PrimitiveType {
        VOID,
        BOOL,
        TRIBOOL,
        INT8,
        UINT8,
        INT16,
        UINT16,
        INT32,
        UINT32,
        INT64,
        UINT64,
        FLOAT,
        DOUBLE,
        STRING,
        JSON,
        BLOB,
        TIMESTAMP,
        INVALID
    }

    export enum MessageType {
        Publish,
        Master,
        Slave,
        Scatter,
        Gather
    }

    export enum GatherFlags {
        NoFlags,
        Finished,
        Ignored,
        Deaf,
        BindingDestroyed,
        ConnectionLost
    }

    export enum Tribool {
        FALSE,
        TRUE,
        UNDEFINED
    }

    export interface DnsLookupInfo {
        addresses: Array<string>;
        hostname: string;
    }

    export interface PrimitiveTypePair {
        first: PrimitiveType;
        second: PrimitiveType;
    }

    export interface StringPair {
        first: string;
        second: string;
    }

    export class Session {
        constructor(name?: string);

        readonly name: string;
        readonly alive: boolean;
        readonly comeAlivePromise: Promise<void>;
        readonly diePromise: Promise<void>;
        readonly loggedInPromise: Promise<void>;
        readonly loggedIn: boolean;
        readonly username: string | null;
        readonly webSessionName: string | null;

        close(): void;
        getVersion(): Promise<string>;
        getServerTime(): Promise<Date>;
        getClientAddress(): Promise<string>;
        logIn(username: string, password: string): Promise<string>;
    }

    export class AccountStorage {
        constructor(session: Session);

        store(variable: string, data: ArrayBuffer): Promise<void>;
        read(variable: string): Promise<ArrayBuffer>;
    }

    export class SessionStorage {
        constructor(session: Session);

        store(variable: string, data: ArrayBuffer): Promise<void>;
        read(variable: string): Promise<ArrayBuffer>;
    }

    export class DnsService {
        constructor(session: Session);

        lookup(hostOrIp: string): Promise<DnsLookupInfo>;
    }

    export abstract class OfficialSignatureHalf {
        readonly raw: number;
        readonly isList: boolean;
        readonly isPair: boolean;
        readonly hasTimestamp: boolean;
        readonly hasData: boolean;
        readonly primitiveTypes: PrimitiveTypePair;
        readonly primitiveTypeNames: StringPair;
        readonly primitiveProtoTypeNames: StringPair;
        readonly isReserved: boolean;

        toString(): string;
    }

    export class Signature {
        constructor(raw: number);

        readonly raw: number;
        readonly isCustom: boolean;
        readonly lowerHalf: OfficialSignatureHalf;
        readonly upperHalf: OfficialSignatureHalf;
        readonly isReserved: boolean;
        readonly representsProtoMessage: boolean;
        readonly bytes: Array<number>;
        readonly bits: Array<number>;
        readonly hex: string;

        toString(): string;
    }

    export class ProtoFile {
        constructor(signature: Signature | number);

        readonly signature: Signature;
        readonly content: string;
        readonly package: string;
        readonly filenameBase: string;
        readonly filename: string;
    }

    export class ProtoCompiler {
        constructor();

        compileToPython(protoFile: ProtoFile): Promise<Map<string, string>>;
        compileToPythonAndDownload(protoFile: ProtoFile): Promise<Map<string, string>>;
        compileToCpp(protoFile: ProtoFile): Promise<Map<string, string>>;
        compileToCppAndDownload(protoFile: ProtoFile): Promise<Map<string, string>>;
        compileToCSharp(protoFile: ProtoFile): Promise<Map<string, string>>;
        compileToCSharpAndDownload(protoFile: ProtoFile): Promise<Map<string, string>>;
    }

    export class TestInterface {
        constructor(session: Session);

        run(command: string): Promise<void>;
        runUntilSuccess(command: string, retries?: number): Promise<void>;
    }

    export abstract class Message {
        serialize(): ArrayBuffer;
        deserialize(buffer: ArrayBuffer): void;

        timestamp?: Long;
        value?: any;
    }

    export class MessageFactory {
        static createMessageClass(type: MessageType, signature: Signature): typeof Message;
        static createMessage(type: MessageType, signature: Signature): Message;
    }

    export abstract class Terminal {
        readonly session: Session;
        readonly name: string;
        readonly signature: Signature;
        readonly alive: boolean;
        readonly comeAlivePromise: Promise<void>;

        toString(): string;
        destroy(): Promise<void>;
    }

    export abstract class PrimitiveTerminal extends Terminal {
    }

    export abstract class ConvenienceTerminal extends Terminal {
    }

    export abstract class Operation {
        readonly terminal: Terminal;
        readonly finished: boolean;
        readonly id: number;

        toString(): string;
    }

    export abstract class ScatterMessage {
        readonly terminal: Terminal;
        readonly message: Message;

        respond(msg: Message): Promise<void>;
        ignore(): Promise<void>;

        toString(): string;
    }

    export abstract class GatherMessage {
        readonly terminal: Terminal;
        readonly operation: Operation;
        readonly flags: GatherFlags;
        readonly message: Message;

        toString(): string;
    }

    export class DeafMuteTerminal extends PrimitiveTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof DeafMuteTerminal;
    }

    export class PublishSubscribeTerminal extends PrimitiveTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof PublishSubscribeTerminal;
        readonly subscribed: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;
        onMessageReceived: (msg: Message) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class CachedPublishSubscribeTerminal extends PrimitiveTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof CachedPublishSubscribeTerminal;
        readonly subscribed: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;
        onMessageReceived: (msg: Message, cached: boolean) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class ScatterGatherTerminal extends PrimitiveTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof ScatterGatherTerminal;
        readonly Operation: typeof Operation;
        readonly ScatterMessage: typeof ScatterMessage;
        readonly GatherMessage: typeof GatherMessage;
        readonly subscribed: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;
        onScatterMessageReceived: (msg: ScatterMessage) => any;

        makeScatterMessage(): Message;
        makeGatherMessage(): Message;
        scatterGather(msg: Message, gatherFn: (msg: GatherMessage) => any): Promise<Operation>;
    }

    export class ProducerTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof ConsumerTerminal;
        readonly subscribed: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class ConsumerTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof ProducerTerminal;
        readonly established: boolean;

        onBindingStateChanged: (subscribed: boolean) => any;
        onMessageReceived: (msg: Message) => any;
    }

    export class CachedProducerTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof CachedConsumerTerminal;
        readonly subscribed: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class CachedConsumerTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof CachedProducerTerminal;
        readonly established: boolean;

        onBindingStateChanged: (established: boolean) => any;
        onMessageReceived: (msg: Message, cached: boolean) => any;
    }

    export class MasterTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof SlaveTerminal;
        readonly subscribed: boolean;
        readonly established: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;
        onBindingStateChanged: (established: boolean) => any;
        onMessageReceived: (msg: Message) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class SlaveTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof MasterTerminal;
        readonly subscribed: boolean;
        readonly established: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;
        onBindingStateChanged: (established: boolean) => any;
        onMessageReceived: (msg: Message) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class CachedMasterTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof CachedSlaveTerminal;
        readonly subscribed: boolean;
        readonly established: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;
        onBindingStateChanged: (established: boolean) => any;
        onMessageReceived: (msg: Message, cached: boolean) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class CachedSlaveTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof CachedMasterTerminal;
        readonly subscribed: boolean;
        readonly established: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;
        onBindingStateChanged: (established: boolean) => any;
        onMessageReceived: (msg: Message, cached: boolean) => any;

        makeMessage(): Message;
        publish(msg: Message): Promise<void>;
    }

    export class ServiceTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof ClientTerminal;
        readonly Request: typeof ScatterMessage;
        readonly established: boolean;

        onBindingStateChanged: (established: boolean) => any;
        onRequestReceived: (msg: ScatterMessage) => any;

        makeGatherMessage(): Message;
    }

    export class ClientTerminal extends ConvenienceTerminal {
        constructor(session: Session, name: string, signature: Signature);

        static readonly CompatibleTerminalType: typeof ServiceTerminal;
        readonly Operation: typeof Operation;
        readonly Response: typeof GatherMessage;
        readonly subscribed: boolean;

        onSubscriptionStateChanged: (subscribed: boolean) => any;

        makeScatterMessage(): Message;
        request(msg: Message, gatherFn: (msg: GatherMessage) => any): Promise<Operation>;
    }

    export class Binding {
        constructor(terminal: PrimitiveTerminal, targets: string);

        readonly terminal: PrimitiveTerminal;
        readonly targets: string;
        readonly alive: boolean;
        readonly comeAlivePromise: Promise<void>;
        readonly established: boolean;

        onBindingStateChanged: (established: boolean) => any;

        toString(): string;
        destroy(): Promise<void>;
    }

    export interface NamelessTerminalInfo {
        type: any;
        signature: Signature;
    }

    export interface TerminalInfo extends NamelessTerminalInfo {
        name: string;
    }

    export interface SubtreeInfo {
        name: string;
        terminals: Array<NamelessTerminalInfo>
    }

    export interface KnownTerminalChangeInfo {
        added: boolean;
        terminal: TerminalInfo;
    }

    export class KnownTerminalsObserver {
        constructor(session: Session);

        readonly session: Session;

        getAll(): Promise<Array<TerminalInfo>>;
        getSubtree(path: string): Promise<Array<SubtreeInfo>>;
        find(nameSubstr: string, caseSensitive?: boolean): Promise<Array<TerminalInfo>>;

        onChanged: (info: KnownTerminalChangeInfo) => any;
    }

    export interface TcpServerInfo {
        id: number,
        address: string,
        port: number
    }

    export interface TcpClientInfo {
        id: number,
        host: string,
        port: number
    }

    export interface ConnectionFactoriesInfo {
        tcpServers: Array<TcpServerInfo>;
        tcpClients: Array<TcpClientInfo>;
    }

    export interface ConnectionInfo {
        factoryId: number,
        connected: boolean,
        description: string,
        remoteVersion: string,
        stateChangeTime: Date
    }

    export class ConnectionsObserver {
        constructor(session: Session);

        readonly session: Session;

        getFactories(): Promise<ConnectionFactoriesInfo>;
        getAll(): Promise<Array<ConnectionInfo>>;

        onChanged: (info: ConnectionInfo) => any;
    }

    export class CustomCommand {
        constructor(session: Session, command: string, args?: Array<string>);

        readonly session: Session;
        readonly command: string;
        readonly args: Array<string>;
        readonly running: boolean;
        readonly exitCode: number | void;
        readonly startedPromise: Promise<void>;
        readonly terminatedPromise: Promise<number>;

        onStdoutRead: (str: string) => any;
        onStderrRead: (str: string) => any;
        onError: (message: string) => any;

        writeStdin(str: string): Promise<void>;
        kill(): Promise<void>;
    }
}

// Type definitions for long.js 3.0.2
// Project: https://github.com/dcodeIO/long.js
// Definitions by: Peter Kooijmans <https://github.com/peterkooijmans/>
// Definitions: https://github.com/DefinitelyTyped/DefinitelyTyped
// Definitions by: Denis Cappellin <http://github.com/cappellin>

// export = Long;
// export as namespace Long;

declare namespace Long {}
declare class Long
{
    /**
     * Constructs a 64 bit two's-complement integer, given its low and high 32 bit values as signed integers. See the from* functions below for more convenient ways of constructing Longs.
     */
    constructor( low: number, high?: number, unsigned?: boolean );

    /**
     * Maximum unsigned value.
     */
    static MAX_UNSIGNED_VALUE: Long;

    /**
     * Maximum signed value.
     */
    static MAX_VALUE: Long;

    /**
     * Minimum signed value.
     */
    static MIN_VALUE: Long;

    /**
     * Signed negative one.
     */
    static NEG_ONE: Long;

    /**
     * Signed one.
     */
    static ONE: Long;

    /**
     * Unsigned one.
     */
    static UONE: Long;

    /**
     * Unsigned zero.
     */
    static UZERO: Long;

    /**
     * Signed zero
     */
    static ZERO: Long;

    /**
     * The high 32 bits as a signed value.
     */
    high: number;

    /**
     * The low 32 bits as a signed value.
     */
    low: number;

    /**
     * Whether unsigned or not.
     */
    unsigned: boolean;

    /**
     * Returns a Long representing the 64 bit integer that comes by concatenating the given low and high bits. Each is assumed to use 32 bits.
     */
    static fromBits( lowBits:number, highBits:number, unsigned?:boolean ): Long;

    /**
     * Returns a Long representing the given 32 bit integer value.
     */
    static fromInt( value: number, unsigned?: boolean ): Long;

    /**
     * Returns a Long representing the given value, provided that it is a finite number. Otherwise, zero is returned.
     */
    static fromNumber( value: number, unsigned?: boolean ): Long;

    /**
     * Returns a Long representation of the given string, written using the specified radix.
     */
    static fromString( str: string, unsigned?: boolean | number, radix?: number ): Long;

    /**
     * Tests if the specified object is a Long.
     */
    static isLong( obj: any ): boolean;

    /**
     * Converts the specified value to a Long.
     */
    static fromValue( val: Long | number | string | {low: number, high: number, unsigned: boolean} ): Long;

    /**
     * Returns the sum of this and the specified Long.
     */
    add( addend: number | Long | string ): Long;

    /**
     * Returns the bitwise AND of this Long and the specified.
     */
    and( other: Long | number | string ): Long;

    /**
     * Compares this Long's value with the specified's.
     */
    compare( other: Long | number | string ): number;

    /**
     * Compares this Long's value with the specified's.
     */
    comp( other: Long | number | string ): number;

    /**
     * Returns this Long divided by the specified.
     */
    divide( divisor: Long | number | string ): Long;

    /**
     * Returns this Long divided by the specified.
     */
    div( divisor: Long | number | string ): Long;

    /**
     * Tests if this Long's value equals the specified's.
     */
    equals( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value equals the specified's.
     */
    eq( other: Long | number | string ): boolean;

    /**
     * Gets the high 32 bits as a signed integer.
     */
    getHighBits(): number;

    /**
     * Gets the high 32 bits as an unsigned integer.
     */
    getHighBitsUnsigned(): number;

    /**
     * Gets the low 32 bits as a signed integer.
     */
    getLowBits(): number;

    /**
     * Gets the low 32 bits as an unsigned integer.
     */
    getLowBitsUnsigned(): number;

    /**
     * Gets the number of bits needed to represent the absolute value of this Long.
     */
    getNumBitsAbs(): number;

    /**
     * Tests if this Long's value is greater than the specified's.
     */
    greaterThan( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value is greater than the specified's.
     */
    gt( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value is greater than or equal the specified's.
     */
    greaterThanOrEqual( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value is greater than or equal the specified's.
     */
    gte( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value is even.
     */
    isEven(): boolean;

    /**
     * Tests if this Long's value is negative.
     */
    isNegative(): boolean;

    /**
     * Tests if this Long's value is odd.
     */
    isOdd(): boolean;

    /**
     * Tests if this Long's value is positive.
     */
    isPositive(): boolean;

    /**
     * Tests if this Long's value equals zero.
     */
    isZero(): boolean;

    /**
     * Tests if this Long's value is less than the specified's.
     */
    lessThan( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value is less than the specified's.
     */
    lt( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value is less than or equal the specified's.
     */
    lessThanOrEqual( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value is less than or equal the specified's.
     */
    lte( other: Long | number | string ): boolean;

    /**
     * Returns this Long modulo the specified.
     */
    modulo( other: Long | number | string ): Long;

    /**
     * Returns this Long modulo the specified.
     */
    mod( other: Long | number | string ): Long;

    /**
     * Returns the product of this and the specified Long.
     */
    multiply( multiplier: Long | number | string ): Long;

    /**
     * Returns the product of this and the specified Long.
     */
    mul( multiplier: Long | number | string ): Long;

    /**
     * Negates this Long's value.
     */
    negate(): Long;

    /**
     * Negates this Long's value.
     */
    neg(): Long;

    /**
     * Returns the bitwise NOT of this Long.
     */
    not(): Long;

    /**
     * Tests if this Long's value differs from the specified's.
     */
    notEquals( other: Long | number | string ): boolean;

    /**
     * Tests if this Long's value differs from the specified's.
     */
    neq( other: Long | number | string ): boolean;

    /**
     * Returns the bitwise OR of this Long and the specified.
     */
    or( other: Long | number | string ): Long;

    /**
     * Returns this Long with bits shifted to the left by the given amount.
     */
    shiftLeft( numBits: number | Long ): Long;

    /**
     * Returns this Long with bits shifted to the left by the given amount.
     */
    shl( numBits: number | Long ): Long;

    /**
     * Returns this Long with bits arithmetically shifted to the right by the given amount.
     */
    shiftRight( numBits: number | Long ): Long;

    /**
     * Returns this Long with bits arithmetically shifted to the right by the given amount.
     */
    shr( numBits: number | Long ): Long;

    /**
     * Returns this Long with bits logically shifted to the right by the given amount.
     */
    shiftRightUnsigned( numBits: number | Long ): Long;

    /**
     * Returns this Long with bits logically shifted to the right by the given amount.
     */
    shru( numBits: number | Long ): Long;

    /**
     * Returns the difference of this and the specified Long.
     */
    subtract( subtrahend: number | Long | string ): Long;

    /**
     * Returns the difference of this and the specified Long.
     */
    sub( subtrahend: number | Long |string ): Long;

    /**
     * Converts the Long to a 32 bit integer, assuming it is a 32 bit integer.
     */
    toInt(): number;

    /**
     * Converts the Long to a the nearest floating-point representation of this value (double, 53 bit mantissa).
     */
    toNumber(): number;

    /**
     * Converts this Long to signed.
     */
    toSigned(): Long;

    /**
     * Converts the Long to a string written in the specified radix.
     */
    toString( radix?: number ): string;

    /**
     * Converts this Long to unsigned.
     */
    toUnsigned(): Long;

    /**
     * Returns the bitwise XOR of this Long and the given one.
     */
    xor( other: Long | number | string ): Long;
}

// Type definitions for bytebuffer.js 5.0.0
// Project: https://github.com/dcodeIO/bytebuffer.js
// Definitions by: Denis Cappellin <http://github.com/cappellin>
// Definitions: https://github.com/DefinitelyTyped/DefinitelyTyped
// Definitions by: SINTEF-9012 <http://github.com/SINTEF-9012>

// import Long = require("long");
/// <reference path="long.d.ts" />

declare namespace ByteBuffer {}
// export = ByteBuffer;
// export as namespace ByteBuffer;

declare class ByteBuffer
{
    /**
     * Constructs a new ByteBuffer.
     */
    constructor( capacity?: number, littleEndian?: boolean, noAssert?: boolean );

    /**
     * Big endian constant that can be used instead of its boolean value. Evaluates to false.
     */
    static BIG_ENDIAN: boolean;

    /**
     * Default initial capacity of 16.
     */
    static DEFAULT_CAPACITY: number;

    /**
     * Default endianess of false for big endian.
     */
    static DEFAULT_ENDIAN: boolean;

    /**
     * Default no assertions flag of false.
     */
    static DEFAULT_NOASSERT: boolean;

    /**
     * Little endian constant that can be used instead of its boolean value. Evaluates to true.
     */
    static LITTLE_ENDIAN: boolean;

    /**
     * Maximum number of bytes required to store a 32bit base 128 variable-length integer.
     */
    static MAX_VARINT32_BYTES: number;

    /**
     * Maximum number of bytes required to store a 64bit base 128 variable-length integer.
     */
    static MAX_VARINT64_BYTES: number;

    /**
     * Metrics representing number of bytes.Evaluates to 2.
     */
    static METRICS_BYTES: number;

    /**
     * Metrics representing number of UTF8 characters.Evaluates to 1.
     */
    static METRICS_CHARS: number;

    /**
     * ByteBuffer version.
     */
    static VERSION: string;

    /**
     * Backing buffer.
     */
    buffer: ArrayBuffer;

    /**
     * Absolute limit of the contained data. Set to the backing buffer's capacity upon allocation.
     */
    limit: number;

    /**
     * Whether to use little endian byte order, defaults to false for big endian.
     */
    littleEndian: boolean;

    /**
     * Marked offset.
     */
    markedOffset: number;

    /**
     * Whether to skip assertions of offsets and values, defaults to false.
     */
    noAssert: boolean;

    /**
     * Absolute read/write offset.
     */
    offset: number;

    /**
     * Data view to manipulate the backing buffer. Becomes null if the backing buffer has a capacity of 0.
     */
    view: DataView;

    /**
     * Allocates a new ByteBuffer backed by a buffer of the specified capacity.
     */
    static allocate( capacity?: number, littleEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Decodes a base64 encoded string to binary like window.atob does.
     */
    static atob( b64: string ): string;

    /**
     * Encodes a binary string to base64 like window.btoa does.
     */
    static btoa( str: string ): string;

    /**
     * Calculates the number of UTF8 bytes of a string.
     */
    static calculateUTF8Byte( str: string ): number;

    /**
     * Calculates the number of UTF8 characters of a string.JavaScript itself uses UTF- 16, so that a string's length property does not reflect its actual UTF8 size if it contains code points larger than 0xFFFF.
     */
    static calculateUTF8Char( str: string ): number;

    /**
     * Calculates the actual number of bytes required to store a 32bit base 128 variable-length integer.
     */
    static calculateVariant32( value: number ): number;

    /**
     * Calculates the actual number of bytes required to store a 64bit base 128 variable-length integer.
     */
    static calculateVariant64( value: number | Long ): number;

    /**
     * Concatenates multiple ByteBuffers into one.
     */
    static concat( buffers: Array<ByteBuffer | ArrayBuffer | Uint8Array | string>, encoding?: string | boolean, litteEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Decodes a base64 encoded string to a ByteBuffer.
     */
    static fromBase64( str: string, littleEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Decodes a binary encoded string, that is using only characters 0x00-0xFF as bytes, to a ByteBuffer.
     */
    static fromBinary( str: string, littleEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Decodes a hex encoded string with marked offsets to a ByteBuffer.
     */
    static fromDebug( str: string, littleEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Decodes a hex encoded string to a ByteBuffer.
     */
    static fromHex( str: string, littleEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Decodes an UTF8 encoded string to a ByteBuffer.
     */
    static fromUTF8( str: string, littleEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Gets the backing buffer type.
     */
    static isByteBuffer( bb: any ): boolean;

    /**
     * Wraps a buffer or a string. Sets the allocated ByteBuffer's ByteBuffer#offset to 0 and its ByteBuffer#limit to the length of the wrapped data.
     * @param buffer Anything that can be wrapped
     * @param encoding String encoding if buffer is a string ("base64", "hex", "binary", defaults to "utf8")
     * @param littleEndian Whether to use little or big endian byte order. Defaults to ByteBuffer.DEFAULT_ENDIAN.
     * @param noAssert Whether to skip assertions of offsets and values. Defaults to ByteBuffer.DEFAULT_NOASSERT.
     */
    static wrap( buffer: ByteBuffer | ArrayBuffer | Uint8Array | string, enc?: string | boolean, littleEndian?: boolean, noAssert?: boolean ): ByteBuffer;

    /**
     * Decodes a zigzag encoded signed 32bit integer.
     */
    static zigZagDecode32( n: number ): number;

    /**
     * Decodes a zigzag encoded signed 64bit integer.
     */
    static zigZagDecode64( n: number | Long ): Long;

    /**
     * Zigzag encodes a signed 32bit integer so that it can be effectively used with varint encoding.
     */
    static zigZagEncode32( n: number ): number;

    /**
     * Zigzag encodes a signed 64bit integer so that it can be effectively used with varint encoding.
     */
    static zigZagEncode64( n: number | Long ): Long;

    /**
     * Switches (to) big endian byte order.
     */
    BE( bigEndian?: boolean ): ByteBuffer;

    /**
     * Switches (to) little endian byte order.
     */
    LE( bigEndian?: boolean ): ByteBuffer;

    /**
     * Appends some data to this ByteBuffer. This will overwrite any contents behind the specified offset up to the appended data's length.
     */
    append( source: ByteBuffer | ArrayBuffer | Uint8Array | string, encoding?: string | number, offset?: number ): ByteBuffer;

    /**
     * Appends this ByteBuffer's contents to another ByteBuffer. This will overwrite any contents behind the specified offset up to the length of this ByteBuffer's data.
     */
    appendTo( target: ByteBuffer, offset?: number ): ByteBuffer;

    /**
     * Enables or disables assertions of argument types and offsets. Assertions are enabled by default but you can opt to disable them if your code already makes sure that everything is valid.
     */
    assert( assert: boolean ): ByteBuffer;

    /**
     * Gets the capacity of this ByteBuffer's backing buffer.
     */
    capacity(): number;

    /**
     * Clears this ByteBuffer's offsets by setting ByteBuffer#offset to 0 and
     * ByteBuffer#limit to the backing buffer's capacity. Discards ByteBuffer#markedOffset.
     */
    clear(): ByteBuffer;

    /**
     * Creates a cloned instance of this ByteBuffer, preset with this ByteBuffer's values for ByteBuffer#offset, ByteBuffer#markedOffset and ByteBuffer#limit.
     */
    clone( copy?: boolean ): ByteBuffer;

    /**
     * Compacts this ByteBuffer to be backed by a ByteBuffer#buffer of its contents' length. Contents are the bytes between ByteBuffer#offset and ByteBuffer#limit. Will set offset = 0 and limit = capacity and adapt ByteBuffer#markedOffset to the same relative position if set.
     */
    compact( begin?: number, end?: number ): ByteBuffer;

    /**
     * Creates a copy of this ByteBuffer's contents. Contents are the bytes between ByteBuffer#offset and ByteBuffer#limit.
     */
    copy( begin?: number, end?: number ): ByteBuffer;

    /**
     * Copies this ByteBuffer's contents to another ByteBuffer. Contents are the bytes between ByteBuffer#offset and ByteBuffer#limit.
     */
    copyTo( target: ByteBuffer, targetOffset?: number, sourceOffset?: number, sourceLimit?: number ): ByteBuffer;

    /**
     * Makes sure that this ByteBuffer is backed by a ByteBuffer#buffer of at least the specified capacity. If the current capacity is exceeded, it will be doubled. If double the current capacity is less than the required capacity, the required capacity will be used instead.
     */
    ensureCapacity( capacity: number ): ByteBuffer;

    /**
     * Overwrites this ByteBuffer's contents with the specified value. Contents are the bytes between ByteBuffer#offset and ByteBuffer#limit.
     */
    fill( value: number | string, begin?: number, end?: number ): ByteBuffer;

    /**
     * Makes this ByteBuffer ready for a new sequence of write or relative read operations. Sets limit = offset and offset = 0. Make sure always to flip a ByteBuffer when all relative read or write operations are complete.
     */
    flip(): ByteBuffer;

    /**
     * Marks an offset on this ByteBuffer to be used later.
     */
    mark( offset?: number ): ByteBuffer;

    /**
     * Sets the byte order.
     */
    order( littleEndian: boolean ): ByteBuffer;

    /**
     * Prepends some data to this ByteBuffer. This will overwrite any contents before the specified offset up to the prepended data's length. If there is not enough space available before the specified offset, the backing buffer will be resized and its contents moved accordingly.
     */
    prepend( source: ByteBuffer | string | ArrayBuffer, encoding?: string | number, offset?: number ): ByteBuffer;

    /**
     * Prepends this ByteBuffer to another ByteBuffer. This will overwrite any contents before the specified offset up to the prepended data's length. If there is not enough space available before the specified offset, the backing buffer will be resized and its contents moved accordingly.
     */
    prependTo( target: ByteBuffer, offset?: number ): ByteBuffer;

    /**
     * Prints debug information about this ByteBuffer's contents.
     */
    printDebug( out?: ( text: string ) => void ): void;

    /**
     * Reads an 8bit signed integer. This is an alias of ByteBuffer#readInt8.
     */
    readByte( offset?: number ): number;

    /**
     * Reads a NULL-terminated UTF8 encoded string. For this to work the string read must not contain any NULL characters itself.
     */
    readCString( offset?: number ): string;

    /**
     * Reads a 64bit float. This is an alias of ByteBuffer#readFloat64.
     */
    readDouble( offset?: number ): number;

    /**
     * Reads a 32bit float. This is an alias of ByteBuffer#readFloat32.
     */
    readFloat( offset?: number ): number;

    /**
     * Reads a 32bit float.
     */
    readFloat32( offset?: number ): number;

    /**
     * Reads a 64bit float.
     */
    readFloat64( offset?: number ): number;

    /**
     * Reads a length as uint32 prefixed UTF8 encoded string.
     */
    readIString( offset?: number ): string;

    /**
     * Reads a 32bit signed integer.This is an alias of ByteBuffer#readInt32.
     */
    readInt( offset?: number ): number;

    /**
     * Reads a 16bit signed integer.
     */
    readInt16( offset?: number ): number;

    /**
     * Reads a 32bit signed integer.
     */
    readInt32( offset?: number ): number;

    /**
     * Reads a 64bit signed integer.
     */
    readInt64( offset?: number ): Long;

    /**
     * Reads an 8bit signed integer.
     */
    readInt8( offset?: number ): number;

    /**
     * Reads a 64bit signed integer. This is an alias of ByteBuffer#readInt64.
     */
    readLong( offset?: number ): Long;

    /**
     * Reads a 16bit signed integer. This is an alias of ByteBuffer#readInt16.
     */
    readShort( offset?: number ): number;

    /**
     * Reads an UTF8 encoded string. This is an alias of ByteBuffer#readUTF8String.
     */
    readString( length: number, metrics?: number, offset?: number ): string;

    /**
     * Reads an UTF8 encoded string.
     */
    readUTF8String( chars: number, offset?: number ): string;

    /**
     * Reads a 16bit unsigned integer.
     */
    readUint16( offset?: number ): number;

    /**
     * Reads a 32bit unsigned integer.
     */
    readUint32( offset?: number ): number;

    /**
     * Reads a 64bit unsigned integer.
     */
    readUint64( offset?: number ): Long;
    /**
     * Reads an 8bit unsigned integer.
     */
    readUint8( offset?: number ): number;

    /**
     * Reads a length as varint32 prefixed UTF8 encoded string.
     */
    readVString( offset?: number ): string;

    /**
     * Reads a 32bit base 128 variable-length integer.
     */
    readVarint32( offset?: number ): number;

    /**
     * Reads a zig-zag encoded 32bit base 128 variable-length integer.
     */
    readVarint32ZiZag( offset?: number ): number;

    /**
     * Reads a 64bit base 128 variable-length integer. Requires Long.js.
     */
    readVarint64( offset?: number ): Long;

    /**
     * Reads a zig-zag encoded 64bit base 128 variable-length integer. Requires Long.js.
     */
    readVarint64ZigZag( offset?: number ): Long;

    /**
     * Gets the number of remaining readable bytes. Contents are the bytes between ByteBuffer#offset and ByteBuffer#limit, so this returns limit - offset.
     */
    remaining(): number;

    /**
     * Resets this ByteBuffer's ByteBuffer#offset. If an offset has been marked through ByteBuffer#mark before, offset will be set to ByteBuffer#markedOffset, which will then be discarded. If no offset has been marked, sets offset = 0.
     */
    reset(): ByteBuffer;

    /**
     * Resizes this ByteBuffer to be backed by a buffer of at least the given capacity. Will do nothing if already that large or larger.
     */
    resize( capacity: number ): ByteBuffer;

    /**
     * Reverses this ByteBuffer's contents
     */
    reverse( begin?: number, end?: number ): ByteBuffer;

    /**
     * Skips the next length bytes. This will just advance
     */
    skip( length: number ): ByteBuffer;

    /**
     * Slices this ByteBuffer by creating a cloned instance with offset = begin and limit = end.
     */
    slice( begin?: number, end?: number ): ByteBuffer;

    /**
     * Returns a raw buffer compacted to contain this ByteBuffer's contents. Contents are the bytes between ByteBuffer#offset and ByteBuffer#limit. Will transparently ByteBuffer#flip this ByteBuffer if offset > limit but the actual offsets remain untouched. This is an alias of ByteBuffer#toBuffer.
     */
    toArrayBuffer( forceCopy?: boolean ): ArrayBuffer;

    /**
     * Encodes this ByteBuffer's contents to a base64 encoded string.
     */
    toBase64( begin?: number, end?: number ): string;

    /**
     * Encodes this ByteBuffer to a binary encoded string, that is using only characters 0x00-0xFF as bytes.
     */
    toBinary( begin?: number, end?: number ): string;

    /**
     * Returns a copy of the backing buffer that contains this ByteBuffer's contents. Contents are the bytes between ByteBuffer#offset and ByteBuffer#limit. Will transparently ByteBuffer#flip this ByteBuffer if offset > limit but the actual offsets remain untouched.
     */
    toBuffer( forceCopy?: boolean ): ArrayBuffer;

    /**
      *Encodes this ByteBuffer to a hex encoded string with marked offsets. Offset symbols are:
     *  < : offset,
     *  ' : markedOffset,
     *  > : limit,
     *  | : offset and limit,
     *  [ : offset and markedOffset,
     *  ] : markedOffset and limit,
     *  ! : offset, markedOffset and limit
     */
    toDebug( columns?: boolean ): string | Array<string>

    /**
     * Encodes this ByteBuffer's contents to a hex encoded string.
     */
    toHex( begin?: number, end?: number ): string;

    /**
     * Converts the ByteBuffer's contents to a string.
     */
    toString( encoding?: string ): string;

    /**
     * Encodes this ByteBuffer's contents between ByteBuffer#offset and ByteBuffer#limit to an UTF8 encoded string.
     */
    toUTF8(): string;

    /**
     * Writes an 8bit signed integer. This is an alias of ByteBuffer#writeInt8.
     */
    writeByte( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a NULL-terminated UTF8 encoded string. For this to work the specified string must not contain any NULL characters itself.
     */
    writeCString( str: string, offset?: number ): ByteBuffer;

    /**
     * Writes a 64bit float. This is an alias of ByteBuffer#writeFloat64.
     */
    writeDouble( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 32bit float. This is an alias of ByteBuffer#writeFloat32.
     */
    writeFloat( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 32bit float.
     */
    writeFloat32( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 64bit float.
     */
    writeFloat64( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a length as uint32 prefixed UTF8 encoded string.
     */
    writeIString( str: string, offset?: number ): ByteBuffer;

    /**
     * Writes a 32bit signed integer. This is an alias of ByteBuffer#writeInt32.
     */
    writeInt( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 16bit signed integer.
     */
    writeInt16( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 32bit signed integer.
     */
    writeInt32( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 64bit signed integer.
     */
    writeInt64( value: number | Long, offset?: number ): ByteBuffer;

    /**
     * Writes an 8bit signed integer.
     */
    writeInt8( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 16bit signed integer. This is an alias of ByteBuffer#writeInt16.
     */
    writeShort( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes an UTF8 encoded string.This is an alias of ByteBuffer#writeUTF8String.
     */
    WriteString( str: string, offset?: number ): ByteBuffer | number;

    /**
     * Writes an UTF8 encoded string.
     */
    writeUTF8String( str: string, offset?: number ): ByteBuffer | number;

    /**
     * Writes a 16bit unsigned integer.
     */
    writeUint16( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 32bit unsigned integer.
     */
    writeUint32( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a 64bit unsigned integer.
     */
    writeUint64( value: number | Long, offset?: number ): ByteBuffer;

    /**
     * Writes an 8bit unsigned integer.
     */
    writeUint8( value: number, offset?: number ): ByteBuffer;

    /**
     * Writes a length as varint32 prefixed UTF8 encoded string.
     */
    writeVString( str: string, offset?: number ): ByteBuffer | number;

    /**
     * Writes a 32bit base 128 variable-length integer.
     */
    writeVarint32( value: number, offset?: number ): ByteBuffer | number;

    /**
     * Writes a zig-zag encoded 32bit base 128 variable-length integer.
     */
    writeVarint32ZigZag( value: number, offset?: number ): ByteBuffer | number;

    /**
     * Writes a 64bit base 128 variable-length integer.
     */
    writeVarint64( value: number | Long, offset?: number ): ByteBuffer;

    /**
     * Writes a zig-zag encoded 64bit base 128 variable-length integer.
     */
    writeVarint64ZigZag( value: number | Long, offset?: number ): ByteBuffer | number;
}

// Type definitions for ProtoBuf.js 5.0.1
// Project: https://github.com/dcodeIO/ProtoBuf.js
// Definitions by: Panu Horsmalahti <https://github.com/panuhorsmalahti>
// Definitions: https://github.com/DefinitelyTyped/DefinitelyTyped

/// <reference path="bytebuffer.d.ts" />
// /// <reference types="bytebuffer" />
// /// <reference types="node" />

declare namespace ProtoBuf {
    // ==========
    // protobufjs/src/ProtoBuf.js

    var Builder: Builder;
    var Long: LongStatic;
    var DotProto: DotProto;
    var Reflect: Reflect;
    var Util: Util;
    var convertFieldsToCamelCase: boolean;

    // var Lang: Lang; TODO: implement interface Lang

    export function loadJson(json: string, builder?: ProtoBuilder | string | {},
        filename?: string | {}): ProtoBuilder;

    export function loadJsonFile(filename: string | {},
        callback?: (error: any, builder: ProtoBuilder) => void,
        builder?: ProtoBuilder): ProtoBuilder;

    export function loadProto(proto: string, builder?: ProtoBuilder | string | {},
        filename?: string | {}): ProtoBuilder;

    export function loadProtoFile(filePath: string | {},
        callback?: (error: any, builder: ProtoBuilder) => void,
        builder?: ProtoBuilder): ProtoBuilder;

    export function newBuilder(options?: {[key: string]: any}): ProtoBuilder;

    export interface LongStatic {
      new(low?: number, high?: number, unsigned?:boolean): Long;

      MAX_UNSIGNED_VALUE: Long;
      MAX_VALUE: Long;
      MIN_VALUE: Long;
      NEG_ONE: Long;
      ONE: Long;
      UONE: Long;
      UZERO: Long;
      ZERO: Long;

      fromBits(lowBits: number, highBits: number, unsigned?: boolean): Long;
      fromInt(value: number, unsigned?: boolean): Long;
      fromNumber(value: number, unsigned?: boolean): Long;
      fromString(str: string, unsigned?: boolean | number, radix?: number): Long;
      fromValue(val: Long | number | string): Long;

      isLong(obj: any): boolean;
    }

    // Based on https://github.com/dcodeIO/Long.js and https://github.com/DefinitelyTyped/DefinitelyTyped/blob/master/long/long.d.ts
    export interface Long {
      high: number;
      low: number;
      unsigned :boolean;

      add(other: Long | number | string): Long;
      and(other: Long | number | string): Long;
      compare(other: Long | number | string): number;
      div(divisor: Long | number | string): Long;
      equals(other: Long | number | string): boolean;
      getHighBits(): number;
      getHighBitsUnsigned(): number;
      getLowBits(): number;
      getLowBitsUnsigned(): number;
      getNumBitsAbs(): number;
      greaterThan(other: Long | number | string): boolean;
      greaterThanOrEqual(other: Long | number | string): boolean;
      isEven(): boolean;
      isNegative(): boolean;
      isOdd(): boolean;
      isPositive(): boolean;
      isZero(): boolean;
      lessThan(other: Long | number | string): boolean;
      lessThanOrEqual(other: Long | number | string): boolean;
      modulo(divisor: Long | number | string): Long;
      multiply(multiplier: Long | number | string): Long;
      negate(): Long;
      not(): Long;
      notEquals(other: Long | number | string): boolean;
      or(other: Long | number | string): Long;
      shiftLeft(numBits: number | Long): Long;
      shiftRight(numBits: number | Long): Long;
      shiftRightUnsigned(numBits: number | Long): Long;
      subtract(other: Long | number | string): Long;
      toInt(): number;
      toNumber(): number;
      toSigned(): Long;
      toString(radix?: number): string;
      toUnsigned(): Long;
      xor(other: Long | number | string): Long;
    }

    // ==========
    // protobufjs/src/ProtoBuf/Builder.js

    export interface Builder {
        new(options?: {[key: string]: any}): ProtoBuilder;
        Message: Message;
        Service: Service;
        isValidMessage(def: {[key: string]: any}): boolean;
        isValidMessageField(def: {[key: string]: any}): boolean;
        isValidEnum(def: {[key: string]: any}): boolean;
        isValidService(def: {[key: string]: any}): boolean;
        isValidExtend(def: {[key: string]: any}): boolean;
    }

    /**
     * TODO: Confirm that message needs no further implementation
     */
    export interface Message {
        new(values?: {[key: string]: any}, var_args?: string[]): Message;
        $add(key: string, value: any, noAssert?: boolean): Message;
        $get<T>(key: string): T;
        $set(key: string | {[key: string]: any}, value?: any | boolean, noAssert?: boolean): void;
        add(key: string, value: any, noAssert?: boolean): Message;
        calculate(): number;
        encode(buffer?: ByteBuffer | boolean, noVerify?: boolean): ByteBuffer;
        encode64(): string;
        encodeAB(): ArrayBuffer;
        encodeNB(): Buffer;
        encodeHex(): string;
        encodeJSON(): string;
        encodeDelimited(buffer?: ByteBuffer | boolean, noVerify?: boolean): ByteBuffer;
        get<T>(key: string, noAssert?: boolean): T;
        set(key: string | {[key: string]: any}, value?: any | boolean, noAssert?: boolean): void;
        toArrayBuffer(): ArrayBuffer;
        toBase64(): string;
        toBuffer(): Buffer;
        toHex(): string;
        toRaw(binaryAsBase64?: boolean, longsAsStrings?: boolean): {[key: string]: any};
        toString(): string;
        [field: string]: any;
  	}

    /**
     * TODO: Implement service interface
     */
    export interface Service {
        new(rpcImpl?: Function): Service;
    }


    // ==========
    // meta objects for constructing protobufs

    export interface ProtoBuilder {
        ns: ReflectNamespace;
        ptr: ReflectNamespace;
        resolved: boolean;
        result: ProtoBuf;
        files: string[];
        importRoot: string;
        options: {[key: string]: any};
        syntax: string;
        reset(): void;
        define(pkg: string, options?: {[key: string]: any}): ProtoBuilder;
        create(defs?: {[key: string]: any}[]): ProtoBuilder;
        resolveAll(): void;
        build(path?: string | [string]): MetaMessage<Message>;
   	    build<T>(path?: string | [string]): MetaMessage<T>;
        lookup(path?: string): ReflectT;
  	}

    export interface ProtoBuf {
        [package: string]: {[key: string]: MetaMessage<Message> | any};
    }

    export interface MetaMessage<T> {
        new(values?: {[key: string]: any}, var_args?: string[]): T & Message;
        decode(buffer: ArrayBuffer | ByteBuffer | Buffer | string, length?: number | string, enc?: string): T & Message;
        decodeDelimited(buffer: ByteBuffer | ArrayBuffer | Buffer | string, enc?: string): T & Message;
        decode64(str: string): T & Message;
        decodeHex(str: string): T & Message;
        decodeJSON(str: string): T & Message;
    }

    // ==========
    // protobufjs/src/ProtoBuf/DotProto.js

    export interface DotProto {
        Parser: Parser;
        Tokenizer: Tokenizer;
    }

    export interface Parser {
        new(proto: string): Parser;
        tn: Tokenizer;
        parse(): MetaProto;
        toString(): string;
    }

    export interface Tokenizer {
        new(proto: string): Tokenizer;
        source: string;
        index: number;
        line: number;
        stack: string[];
        readingString: boolean;
        stringEndsWith: string;
        next(): string;
        peek(): string;
        toString(): string;
    }

    // ==========
    // proto meta information returned by the Parser

    export interface MetaProto {
        package: string;
        messages: ProtoMessage[];
        enums: ProtoEnum[];
        imports: string[];
        options: {[key: string]: any};
        services: ProtoService[];
    }

    export interface ProtoEnum {
        name: string;
        values: ProtoEnumValue[];
        options: {[key: string]: any};
    }

    export interface ProtoEnumValue {
        name: string;
        id: string;
    }

    export interface ProtoField {
        rule: string;
        options: {[key: string]: any};
        type: string;
        name: string;
        id: number;
        oneof?: string;
    }

    export interface ProtoMessage {
        name: string;
        isGroup?: boolean;
        fields: ProtoField[];
        enums: ProtoEnum[];
        messages: ProtoMessage[];
        options: {[key: string]: any};
        oneofs: {[key: string]:number[]};
    }

    export interface ProtoRpcService {
        request: string;
        response: string;
        options: {[key: string]: any};
    }

    export interface ProtoService {
        name: string;
        rpc: {[key: string]:ProtoRpcService};
        options: {[key: string]: any};
    }

    // ==========
    // protobufjs/src/ProtoBuf/Util.js

    export interface Util {
        IS_NODE: boolean
        fetch(path: string, callback?: (data: string) => any): string;
        toCamelCase(str: string): string;
        XHR(): XMLHttpRequest;
    }

    // ==========
    // protobufjs/src/ProtoBuf/Reflect.js

    export interface Reflect {
        T: ReflectT;
        Namespace: ReflectNamespace;
        Message: ReflectMessage;
        Enum: ReflectEnum;
        Extension: ReflectExtension;
        Service: ReflectService;
    }

    export interface ReflectT {
        new(builder?: ProtoBuilder, parent?: ReflectT, name?: string): ReflectT;
        builder: ProtoBuilder;
        parent: ReflectT;
        name: string;
        fqn(): string;
        toString(includeClass?: boolean): string;
    }

    export interface ReflectNamespace extends ReflectT {
        new(builder?: ProtoBuilder, parent?: ReflectNamespace, name?: string,
            options?: {[key: string]: any}): ReflectNamespace;
        className: string;
        children: ReflectT[];
        options: {[key: string]: any};
        syntax: string;
        getChildren(type?: ReflectT): ReflectT[];
        addChild(child: ReflectT): void;
        getChild(nameOrId?: string | number): ReflectT;
        resolve(qn: string, excludeFields?: boolean): ReflectNamespace;
        build(): ProtoBuf;
        buildOpt(): {[key: string]: any};
        getOption(name?: string): any;
    }

    export interface ReflectMessage extends ReflectNamespace {
        new(builder?: ProtoBuilder, parent?: ReflectNamespace, name?: string,
            options?: {[key: string]: any}, isGroup?: boolean): ReflectMessage;
        Field: ReflectField; // NOTE: only for new ProtoBuf.Reflect.Message.Field();
        ExtensionField: ReflectExtensionField; // NOTE: only for
                                          // new ProtoBuf.Reflect.Message.ExtensionField();
        OneOf: ReflectOneOf; // NOTE: only for new ProtoBuf.Reflect.Message.OneOf();
        extensions: number[];
        clazz(): MetaMessage<Message>;
        isGroup: boolean;
        build(rebuild?: boolean): MetaMessage<Message>|any;
        build<T>(rebuild?: boolean): MetaMessage<T>|any;
        encode(message: Message, buffer: Buffer, noVerify?: boolean): Buffer;
        calculate(message: Message): number;
        decode(buffer: Buffer, length?: number, expectedGroupEndId?: number): Message;
    }

    export interface ReflectEnum extends ReflectNamespace {
        new(builder?: ProtoBuilder, parent?: ReflectT, name?: string,
            options?: {[key: string]: any}): ReflectEnum;
        Value: ReflectValue; // NOTE: only for new ProtoBuf.Reflect.Enum.Value();
        object: {[key: string]:number};
        build(): {[key: string]: any};
    }

    export interface ReflectExtension extends ReflectT {
        new(builder?: ProtoBuilder, parent?: ReflectT, name?: string,
            field?: ReflectField): ReflectExtension;
        field: ReflectField;
    }

    export interface ReflectService extends ReflectNamespace {
        new(): ReflectService;
        Method: ReflectMethod; // NOTE: only for new ProtoBuf.Reflect.Service.Method();
        RPCMethod: ReflectRPCMethod; // NOTE: only for new ProtoBuf.Reflect.Service.RPCMethod();
        clazz(): Function;
        build(rebuild?: boolean): Function|any;
    }

    // TODO: check that the runtime instance of this type reflects this definition
    export interface ReflectField extends ReflectT {
        new(builder: ProtoBuilder, message: ReflectMessage, rule: string, type: string,
            name: string, id: number, options: {[key: string]: any}, oneof: ReflectOneOf): ReflectField;
        className: string;
        required: boolean;
        repeated: boolean;
        type: string | WireTuple;
        resolvedType: ReflectT;
        id: number;
        options: {[key: string]: any};
        defaultValue: any;
        oneof: ReflectOneOf;
        originalName: string;
        build(): {[key: string]: any};
        mkLong(value: any, unsigned?: boolean): number;
        verifyValue(value: any, skipRepeated?: boolean): any;
        encode(value: any, buffer: Buffer): Buffer;
        encodeValue(value: any, buffer: Buffer): Buffer;
        calculate(value: any): number;
        calculateValue(value: any): number;
        decode(wireType: number, buffer: Buffer, skipRepeated?: boolean): any;
    }

    export interface WireTuple {
      name: string;
      wireType: number;
    }

    // TODO: check that the runtime instance of this type reflects this definition
    export interface ReflectExtensionField extends ReflectField {
        new(builder: ProtoBuilder, message: ReflectMessage, rule: string, type: string,
            name: string, id: number, options: {[key: string]: any}): ReflectExtensionField;
        extension: ReflectExtension;
    }

    export interface ReflectOneOf extends ReflectT {
        new(builder?: ProtoBuilder, message?: ReflectMessage, name?: string): ReflectOneOf;
        fields: ReflectField[];
    }

    export interface ReflectValue extends ReflectT {
        new(builder?: ProtoBuilder, enm?: ReflectEnum, name?: string, id?: number): ReflectValue;
        className: string;
        id: number;
    }

    export interface ReflectMethod extends ReflectT {
        new(builder?: ProtoBuilder, svc?: ReflectService, name?: string,
            options?: {[key: string]: any}): ReflectMethod;
        className: string;
        options: {[key: string]: any};
        buildOpt(): {[key: string]: any};
    }

    export interface ReflectRPCMethod extends ReflectMethod {
        new(builder?: ProtoBuilder, svc?: ReflectService, name?: string, request?: string,
            response?: string, options?: {[key: string]: any}): ReflectRPCMethod;
        requestName: string;
        responseName: string;
        resolvedRequestType: ReflectMessage;
        resolvedResponseType: ReflectMessage;
    }

}

declare module "protobufjs" {
    export = ProtoBuf;
}

declare module "protobufjs/dist/protobuf-light" {
    export = ProtoBuf;
}
