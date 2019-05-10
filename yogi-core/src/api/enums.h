/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../config.h"
#include "../utils/types.h"

namespace api {

enum ConfigurationFlags {
  kNoFlags = YOGI_CFG_NONE,
  kDisableVariables = YOGI_CFG_DISABLE_VARIABLES,
  kMutableCmdLine = YOGI_CFG_MUTABLE_CMD_LINE,
  kAllFlags = kDisableVariables | kMutableCmdLine,
};

YOGI_DEFINE_FLAG_OPERATORS(ConfigurationFlags)

enum BranchEvents {
  kNoEvent = 0,
  kBranchDiscoveredEvent = YOGI_BEV_BRANCH_DISCOVERED,
  kBranchQueriedEvent = YOGI_BEV_BRANCH_QUERIED,
  kConnectFinishedEvent = YOGI_BEV_CONNECT_FINISHED,
  kConnectionLostEvent = YOGI_BEV_CONNECTION_LOST,
  kAllEvents = YOGI_BEV_ALL,
};

YOGI_DEFINE_FLAG_OPERATORS(BranchEvents)

enum CommandLineOptions {
  kNoOptions = YOGI_CLO_NONE,
  kLoggingOptions = YOGI_CLO_LOGGING,
  kBranchNameOption = YOGI_CLO_BRANCH_NAME,
  kBranchDescriptionOption = YOGI_CLO_BRANCH_DESCRIPTION,
  kBranchNetworkOption = YOGI_CLO_BRANCH_NETWORK,
  kBranchPasswordOption = YOGI_CLO_BRANCH_PASSWORD,
  kBranchPathOption = YOGI_CLO_BRANCH_PATH,
  kBranchAdvInterfacesOption = YOGI_CLO_BRANCH_ADV_IFS,
  kBranchAdvAddressOption = YOGI_CLO_BRANCH_ADV_ADDR,
  kBranchAdvPortOption = YOGI_CLO_BRANCH_ADV_PORT,
  kBranchAdvIntervalOption = YOGI_CLO_BRANCH_ADV_INT,
  kBranchTimeoutOption = YOGI_CLO_BRANCH_TIMEOUT,
  kBranchGhostModeOption = YOGI_CLO_BRANCH_GHOST_MODE,
  kFileOption = YOGI_CLO_FILES,
  kFileRequiredOption = YOGI_CLO_FILES_REQUIRED,
  kOverrideOption = YOGI_CLO_OVERRIDES,
  kVariableOption = YOGI_CLO_VARIABLES,
  kAllOptions = YOGI_CLO_ALL,
};

YOGI_DEFINE_FLAG_OPERATORS(CommandLineOptions)

enum Verbosity {
  kNone = YOGI_VB_NONE,
  kFatal = YOGI_VB_FATAL,
  kError = YOGI_VB_ERROR,
  kWarning = YOGI_VB_WARNING,
  kInfo = YOGI_VB_INFO,
  kDebug = YOGI_VB_DEBUG,
  kTrace = YOGI_VB_TRACE,
};

enum Signals {
  kNoSignal = YOGI_SIG_NONE,
  kSigInt = YOGI_SIG_INT,
  kSigTerm = YOGI_SIG_TERM,
  kSigUsr1 = YOGI_SIG_USR1,
  kSigUsr2 = YOGI_SIG_USR2,
  kSigUsr3 = YOGI_SIG_USR3,
  kSigUsr4 = YOGI_SIG_USR4,
  kSigUsr5 = YOGI_SIG_USR5,
  kSigUsr6 = YOGI_SIG_USR6,
  kSigUsr7 = YOGI_SIG_USR7,
  kSigUsr8 = YOGI_SIG_USR8,
  kAllSignals = YOGI_SIG_ALL,
};

YOGI_DEFINE_FLAG_OPERATORS(Signals)

enum Encoding {
  kJson = YOGI_ENC_JSON,
  kMsgPack = YOGI_ENC_MSGPACK,
};

}  // namespace api
