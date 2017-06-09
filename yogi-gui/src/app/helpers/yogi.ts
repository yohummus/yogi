export function makeTerminalTypeString(terminalType: any): string {
  switch (terminalType) {
    case yogi.DeafMuteTerminal:
      return 'Deaf-Mute (DM) Terminal';

    case yogi.PublishSubscribeTerminal:
      return 'Publish-Subscribe (PS) Terminal';

    case yogi.CachedPublishSubscribeTerminal:
      return 'Cached Publish-Subscribe (CPS) Terminal';

    case yogi.ScatterGatherTerminal:
      return 'Scatter-Gather (SG) Terminal';

    case yogi.ProducerTerminal:
      return 'Producer (PC) Terminal';

    case yogi.ConsumerTerminal:
      return 'Consumer (PC) Terminal';

    case yogi.CachedProducerTerminal:
      return 'Cached Producer (CPC) Terminal';

    case yogi.CachedConsumerTerminal:
      return 'Cached Consumer (CPC) Terminal';

    case yogi.MasterTerminal:
      return 'Master (MS) Terminal';

    case yogi.SlaveTerminal:
      return 'Slave (MS) Terminal';

    case yogi.CachedMasterTerminal:
      return 'Cached Master (CMS) Terminal';

    case yogi.CachedSlaveTerminal:
      return 'Cached Slave (CMS) Terminal';

    case yogi.ServiceTerminal:
      return 'Service (SC) Terminal';

    case yogi.ClientTerminal:
      return 'Client (SC) Terminal';

    default:
      throw new Error(`Unknown terminal type: ${terminalType}.`);
  }
}

export function makeSignatureString(signature: yogi.Signature, terminalType: any): string {
  if (signature.isCustom) {
    return `Custom Signature ${signature.hex}`;
  }
  else if (signature.isReserved) {
    return 'Reserved Signature ${signature.hex}';
  }
  else {
    switch (terminalType) {
      case yogi.DeafMuteTerminal:
        return 'Official Signature ${signature.hex}';

      case yogi.ScatterGatherTerminal:
      case yogi.ServiceTerminal:
      case yogi.ClientTerminal:
        return `Official Signature ${signature.hex}: Scatter Messages contain`
             + ` ${signature.upperHalf} and Gather Messages contain ${signature.lowerHalf}`;

      default:
        return `Official Signature ${signature.hex}: Publish Messages contain`
             + ` ${signature.lowerHalf}`;
    }
  }
}
