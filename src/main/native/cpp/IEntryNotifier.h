/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2017. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#ifndef NT_IENTRYNOTIFIER_H_
#define NT_IENTRYNOTIFIER_H_

#include <climits>

#include "ntcore_cpp.h"

namespace nt {

class IEntryNotifier {
 public:
  IEntryNotifier() = default;
  IEntryNotifier(const IEntryNotifier&) = delete;
  IEntryNotifier& operator=(const IEntryNotifier&) = delete;
  virtual ~IEntryNotifier() = default;
  virtual bool local_notifiers() const = 0;
  virtual void NotifyEntry(unsigned int local_id, StringRef name,
                           std::shared_ptr<Value> value, unsigned int flags,
                           unsigned int only_listener = UINT_MAX) = 0;
};

}  // namespace nt

#endif  // NT_IENTRYNOTIFIER_H_
