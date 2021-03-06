// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "command_line.hh"

#include <algorithm>
#include <ostream>

#include "basictypes.hh"
#include "logging.hh"
#include "string_split.hh"
#include "string_util.hh"

CommandLine* CommandLine::current_process_commandline_ = NULL;

namespace {
const CommandLine::CharType kSwitchTerminator[] = "--";
const CommandLine::CharType kSwitchValueSeparator[] = "=";
// Since we use a lazy match, make sure that longer versions (like "--") are
// listed before shorter versions (like "-") of similar prefixes.
const CommandLine::CharType* const kSwitchPrefixes[] = {"--", "-"};

size_t GetSwitchPrefixLength(const CommandLine::StringType& string) {
  for (size_t i = 0; i < arraysize(kSwitchPrefixes); ++i) {
    CommandLine::StringType prefix(kSwitchPrefixes[i]);
    if (string.compare(0, prefix.length(), prefix) == 0)
      return prefix.length();
  }
  return 0;
}

// Fills in |switch_string| and |switch_value| if |string| is a switch.
// This will preserve the input switch prefix in the output |switch_string|.
bool IsSwitch(const CommandLine::StringType& string,
              CommandLine::StringType* switch_string,
              CommandLine::StringType* switch_value) {
  switch_string->clear();
  switch_value->clear();
  if (GetSwitchPrefixLength(string) == 0)
    return false;

  const size_t equals_position = string.find(kSwitchValueSeparator);
  *switch_string = string.substr(0, equals_position);
  if (equals_position != CommandLine::StringType::npos)
    *switch_value = string.substr(equals_position + 1);
  return true;
}

// Append switches and arguments, keeping switches before arguments.
void AppendSwitchesAndArguments(CommandLine& command_line,
                                const CommandLine::StringVector& argv) {
  bool parse_switches = true;
  for (size_t i = 1; i < argv.size(); ++i) {
    CommandLine::StringType arg = argv[i];
    TrimWhitespace(arg, TRIM_ALL, &arg);

    CommandLine::StringType switch_string;
    CommandLine::StringType switch_value;
    parse_switches &= (arg != kSwitchTerminator);
    if (parse_switches && IsSwitch(arg, &switch_string, &switch_value)) {
      command_line.AppendSwitchNative(switch_string, switch_value);
    } else {
      command_line.AppendArgNative(arg);
    }
  }
}

// Lowercase switches for backwards compatiblity *on Windows*.
std::string LowerASCIIOnWindows(const std::string& string) {
  return string;
}

}  // namespace

CommandLine::CommandLine(NoProgram no_program)
    : argv_(1),
      begin_args_(1) {
}

CommandLine::CommandLine(int argc, const CommandLine::CharType* const* argv)
    : argv_(1),
      begin_args_(1) {
  InitFromArgv(argc, argv);
}

CommandLine::CommandLine(const StringVector& argv)
    : argv_(1),
      begin_args_(1) {
  InitFromArgv(argv);
}

CommandLine::~CommandLine() {
}

// static
void CommandLine::Init(int argc, const char* const* argv) {
  if (current_process_commandline_) {
    // If this is intentional, Reset() must be called first. If we are using
    // the shared build mode, we have to share a single object across multiple
    // shared libraries.
    return;
  }

  current_process_commandline_ = new CommandLine(NO_PROGRAM);
  current_process_commandline_->InitFromArgv(argc, argv);
}

// static
void CommandLine::Reset() {
  DCHECK(current_process_commandline_);
  delete current_process_commandline_;
  current_process_commandline_ = NULL;
}

// static
CommandLine* CommandLine::ForCurrentProcess() {
  DCHECK(current_process_commandline_);
  return current_process_commandline_;
}

// static
CommandLine CommandLine::FromString(const std::string& command_line) {
  CommandLine cmd(NO_PROGRAM);
  cmd.ParseFromString(command_line);
  return cmd;
}

void CommandLine::InitFromArgv(int argc,
                               const CommandLine::CharType* const* argv) {
  StringVector new_argv;
  for (int i = 0; i < argc; ++i)
    new_argv.push_back(argv[i]);
  InitFromArgv(new_argv);
}

void CommandLine::InitFromArgv(const StringVector& argv) {
  argv_ = StringVector(1);
  begin_args_ = 1;
  AppendSwitchesAndArguments(*this, argv);
}

CommandLine::StringType CommandLine::GetCommandLineString() const {
  StringType string(argv_[0]);
  // Append switches and arguments.
  bool parse_switches = true;
  for (size_t i = 1; i < argv_.size(); ++i) {
    CommandLine::StringType arg = argv_[i];
    CommandLine::StringType switch_string;
    CommandLine::StringType switch_value;
    parse_switches &= arg != kSwitchTerminator;
    string.append(StringType(" "));
    if (parse_switches && IsSwitch(arg, &switch_string, &switch_value)) {
      string.append(switch_string);
      if (!switch_value.empty()) {
        string.append(kSwitchValueSeparator + switch_value);
      }
    }
    else {
      string.append(arg);
    }
  }
  return string;
}

bool CommandLine::HasSwitch(const std::string& switch_string) const {
  return switches_.find(LowerASCIIOnWindows(switch_string)) != switches_.end();
}

std::string CommandLine::GetSwitchValueASCII(
    const std::string& switch_string) const {
  StringType value = GetSwitchValueNative(switch_string);
  return value;
}

CommandLine::StringType CommandLine::GetSwitchValueNative(
    const std::string& switch_string) const {
  SwitchMap::const_iterator result = switches_.end();
  result = switches_.find(LowerASCIIOnWindows(switch_string));
  return result == switches_.end() ? StringType() : result->second;
}

void CommandLine::AppendSwitch(const std::string& switch_string) {
  AppendSwitchNative(switch_string, StringType());
}

void CommandLine::AppendSwitchNative(const std::string& switch_string,
                                     const CommandLine::StringType& value) {
  std::string switch_key(LowerASCIIOnWindows(switch_string));
  StringType combined_switch_string(switch_string);
  size_t prefix_length = GetSwitchPrefixLength(combined_switch_string);
  switches_[switch_key.substr(prefix_length)] = value;
  // Preserve existing switch prefixes in |argv_|; only append one if necessary.
  if (prefix_length == 0)
    combined_switch_string = kSwitchPrefixes[0] + combined_switch_string;
  if (!value.empty())
    combined_switch_string += kSwitchValueSeparator + value;
  // Append the switch and update the switches/arguments divider |begin_args_|.
  argv_.insert(argv_.begin() + begin_args_++, combined_switch_string);
}

void CommandLine::AppendSwitchASCII(const std::string& switch_string,
                                    const std::string& value_string) {
  AppendSwitchNative(switch_string, value_string);
}

void CommandLine::CopySwitchesFrom(const CommandLine& source,
                                   const char* const switches[],
                                   size_t count) {
  for (size_t i = 0; i < count; ++i) {
    if (source.HasSwitch(switches[i]))
      AppendSwitchNative(switches[i], source.GetSwitchValueNative(switches[i]));
  }
}

CommandLine::StringVector CommandLine::GetArgs() const {
  // Gather all arguments after the last switch (may include kSwitchTerminator).
  StringVector args(argv_.begin() + begin_args_, argv_.end());
  // Erase only the first kSwitchTerminator (maybe "--" is a legitimate page?)
  StringVector::iterator switch_terminator =
      std::find(args.begin(), args.end(), kSwitchTerminator);
  if (switch_terminator != args.end())
    args.erase(switch_terminator);
  return args;
}

void CommandLine::AppendArg(const std::string& value) {
  AppendArgNative(value);
}

void CommandLine::AppendArgNative(const CommandLine::StringType& value) {
  argv_.push_back(value);
}

void CommandLine::AppendArguments(const CommandLine& other,
                                  bool include_program) {
  AppendSwitchesAndArguments(*this, other.argv());
}

void CommandLine::PrependWrapper(const CommandLine::StringType& wrapper) {
  if (wrapper.empty())
    return;
  // The wrapper may have embedded arguments (like "gdb --args"). In this case,
  // we don't pretend to do anything fancy, we just split on spaces.
  StringVector wrapper_argv;
  chromium::SplitString(wrapper, ' ', &wrapper_argv);
  // Prepend the wrapper and update the switches/arguments |begin_args_|.
  argv_.insert(argv_.begin(), wrapper_argv.begin(), wrapper_argv.end());
  begin_args_ += wrapper_argv.size();
}

void CommandLine::ParseFromString(const std::string& command_line) {
  std::string command_line_string;
  TrimWhitespace(command_line, TRIM_ALL, &command_line_string);
  if (command_line_string.empty())
    return;

  StringVector args;
  chromium::SplitString(command_line, ' ', &args);
  InitFromArgv(args);
}

/* eof */
