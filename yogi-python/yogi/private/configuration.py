# This file is part of the Yogi distribution https://github.com/yohummus/yogi.
# Copyright (c) 2018 Johannes Bergmann.
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

from .object import Object
from .errors import DescriptiveFailure, FailureException, ErrorCode, \
    api_result_handler, run_with_discriptive_failure_awareness
from .library import yogi
from .json_view import JsonView

from enum import IntEnum
import json as json_module
from typing import List, Union, Optional, Dict
from ctypes import c_int, c_char_p, c_char, c_void_p, POINTER, byref, \
    create_string_buffer, sizeof


class ConfigurationFlags(IntEnum):
    """Flags used to change a configuration object's behaviour.

    Attributes:
        NONE              No flags.
        DISABLE_VARIABLES Disables support for variables in the configuration.
        MUTABLE_CMDLINE   Makes configuration options given directly on the
                          command line overridable.
    """
    NONE = 0
    DISABLE_VARIABLES = (1 << 0)
    MUTABLE_CMD_LINE = (1 << 1)


class CommandLineOptions(IntEnum):
    """Flags used to adjust how command line options are parsed.

    Attributes:
        NONE               No options.
        LOGGING            Include logging configuration for file logging.
        BRANCH_NAME        Include branch name configuration.
        BRANCH_DESCRIPTION Include branch description configuration.
        BRANCH_NETWORK     Include network name configuration.
        BRANCH_PASSWORD    Include network password configuration.
        BRANCH_PATH        Include branch path configuration.
        BRANCH_ADV_IFS     Include branch advertising interfaces configuration.
        BRANCH_ADV_ADDR    Include branch advertising address configuration.
        BRANCH_ADV_PORT    Include branch advertising port configuration.
        BRANCH_ADV_INT     Include branch advertising interval configuration.
        BRANCH_TIMEOUT     Include branch timeout configuration.
        BRANCH_GHOST_MDOE  Include ghost mode configuration.
        FILES              Parse configuration files given on the command
                           line.
        FILES_REQUIRED     Same as FILES but at least one configuration file
                           must be given.
        OVERRIDES          Allow overriding arbitrary configuration sections.
        VARIABLES          Allow setting variables via a dedicated switch.
        BRANCH_ALL         Combination of all branch flags.
        ALL                Combination of all flags.
    """
    NONE = 0
    LOGGING = (1 << 0)
    BRANCH_NAME = (1 << 1)
    BRANCH_DESCRIPTION = (1 << 2)
    BRANCH_NETWORK = (1 << 3)
    BRANCH_PASSWORD = (1 << 4)
    BRANCH_PATH = (1 << 5)
    BRANCH_ADV_IFS = (1 << 6)
    BRANCH_ADV_ADDR = (1 << 7)
    BRANCH_ADV_PORT = (1 << 8)
    BRANCH_ADV_INT = (1 << 9)
    BRANCH_TIMEOUT = (1 << 10)
    BRANCH_GHOST_MODE = (1 << 11)
    FILES = (1 << 12)
    FILES_REQUIRED = (1 << 13)
    OVERRIDES = (1 << 14)
    VARIABLES = (1 << 15)
    BRANCH_ALL = BRANCH_NAME | BRANCH_DESCRIPTION | BRANCH_NETWORK \
        | BRANCH_PASSWORD | BRANCH_PATH | BRANCH_ADV_IFS | BRANCH_ADV_ADDR \
        | BRANCH_ADV_PORT | BRANCH_ADV_INT | BRANCH_TIMEOUT \
        | BRANCH_GHOST_MODE
    ALL = LOGGING | BRANCH_ALL | FILES | FILES_REQUIRED | OVERRIDES \
        | VARIABLES


yogi.YOGI_ConfigurationCreate.restype = api_result_handler
yogi.YOGI_ConfigurationCreate.argtypes = [POINTER(c_void_p), c_int]

yogi.YOGI_ConfigurationUpdateFromCommandLine.restype = int
yogi.YOGI_ConfigurationUpdateFromCommandLine.argtypes = [
    c_void_p, c_int, POINTER(POINTER(c_char)), c_int, c_char_p, c_int]

yogi.YOGI_ConfigurationUpdateFromJson.restype = int
yogi.YOGI_ConfigurationUpdateFromJson.argtypes = [c_void_p, c_char_p,
                                                  c_char_p, c_int]

yogi.YOGI_ConfigurationUpdateFromFile.restype = int
yogi.YOGI_ConfigurationUpdateFromFile.argtypes = [c_void_p, c_char_p,
                                                  c_char_p, c_int]

yogi.YOGI_ConfigurationDump.restype = api_result_handler
yogi.YOGI_ConfigurationDump.argtypes = [c_void_p, c_char_p, c_int, c_int,
                                        c_int]

yogi.YOGI_ConfigurationWriteToFile.restype = api_result_handler
yogi.YOGI_ConfigurationWriteToFile.argtypes = [c_void_p, c_char_p, c_int,
                                               c_int]


class Configuration(Object):
    """Stores program parameters from different sources.

    A configuration represents a set of parameters that usually remain constant
    throughout the runtime of a program. Parameters can come from different
    sources such as the command line or a file. Configurations are used for
    other parts of the library such as application objects, however, they are
    also intended to store user-defined parameters.
    """

    def __init__(self, flags: ConfigurationFlags = ConfigurationFlags.NONE, *,
                 json: Union[JsonView, str, object] = None):
        """Create a configuration.

        Args:
            flags: Flags for changing the configuration's behaviour.
            json:  Initial JSON to use.
        """
        handle = c_void_p()
        yogi.YOGI_ConfigurationCreate(byref(handle), flags)
        Object.__init__(self, handle)
        self._flags = flags

        if json is not None:
            self.update_from_json(json)

    @property
    def flags(self) -> ConfigurationFlags:
        """Configuration flags."""
        return self._flags

    def update_from_command_line(self, argv: List[str],
                                 options: CommandLineOptions
                                 = ConfigurationFlags.NONE) -> None:
        """Updates the configuration from command line options.

        If parsing the command line, files or any given JSON string fails, or
        if help is requested (e.g. by using the --help switch) then a
        DescriptiveFailure exception will be raised containing detailed
        information about the error or the help text.

        Args:
            argv:    List of command line arguments including the script name.
            options: Options to provide on the command line.
        """
        args = (POINTER(c_char) * (len(argv)))()
        for i, arg in enumerate(argv):
            args[i] = create_string_buffer(arg.encode())

        run_with_discriptive_failure_awareness(
            lambda err: yogi.YOGI_ConfigurationUpdateFromCommandLine(
                self._handle, len(args), args, options, err, sizeof(err)))

    def update_from_json(self, json: Union[JsonView, str, object]) -> None:
        """Updates the configuration from a JSON object or a JSON object
        serialized to a string.

        If parsing fails then a DescriptiveFailure exception will be raised
        containing detailed information about the error.

        Args:
            json: JsonView, serializable object or already serialized object.
        """
        if not isinstance(json, JsonView):
            json = JsonView(json)

        run_with_discriptive_failure_awareness(
            lambda err: yogi.YOGI_ConfigurationUpdateFromJson(
                self._handle, json.data.obj, err, sizeof(err)))

    def update_from_file(self, filename: str) -> None:
        """Updates the configuration from a JSON file.

        If parsing the file fails then a DescriptiveFailure exception will be
        raised containing detailed information about the error.

        Args:
            filename: Path to the JSON file.
        """
        run_with_discriptive_failure_awareness(
            lambda err: yogi.YOGI_ConfigurationUpdateFromFile(
                self._handle, filename.encode(), err, sizeof(err)))

    def dump(self, resolve_variables: Optional[bool] = None,
             indentation: Optional[int] = None) -> str:
        """Retrieves the configuration as a JSON-formatted string.

        Args:
            resolve_variables: Resolve all configuration variables. If this is
                               None then variables will be resolved if and
                               only if the configuration supports variables.
            indentation:       Number of space characters to use for
                               indentation. A value of None uses no spaces
                               and omits new lines as well.

        Returns:
            The configuration as a JSON-formatted string.
        """
        if resolve_variables is None:
            if self._flags & ConfigurationFlags.DISABLE_VARIABLES:
                resolve_variables = False
            else:
                resolve_variables = True

        if indentation is None:
            indentation = -1

        s = create_string_buffer(256)
        while True:
            try:
                yogi.YOGI_ConfigurationDump(self._handle, s, sizeof(s),
                                            resolve_variables, indentation)
                break
            except FailureException as e:
                if e.failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                    s = create_string_buffer(sizeof(s) * 2)
                else:
                    raise

        return s.value.decode("utf-8")

    def to_json(self, resolve_variables: Optional[bool] = None) \
            -> Dict[str, object]:
        """Retrieves the configuration as a JSON object.

        Args:
            resolve_variables: Resolve all configuration variables. If this is
                               None then variables will be resolved if and
                               only if the configuration supports variables.

        Returns:
            Dictionary representing the configuration.
        """
        return json_module.loads(self.dump(resolve_variables))

    def write_to_file(self, filename: str,
                      resolve_variables: Optional[bool] = None,
                      indentation: Optional[int] = None) -> None:
        """Writes the configuration to a file in JSON format.

        This is useful for debugging purposes.

        Args:
            filename:          Path to the output file.
            resolve_variables: Resolve all configuration variables. If this is
                               None then variables will be resolved if and
                               only if the configuration supports variables.
            indentation:       Number of space characters to use for
                               indentation. A value of None uses no spaces
                               and omits new lines as well.
        """
        if resolve_variables is None:
            if self._flags & ConfigurationFlags.DISABLE_VARIABLES:
                resolve_variables = False
            else:
                resolve_variables = True

        if indentation is None:
            indentation = -1

        yogi.YOGI_ConfigurationWriteToFile(
            self._handle, filename.encode(), resolve_variables,
            indentation)
