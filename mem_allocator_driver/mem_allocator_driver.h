// Copyright (c) 2015-2016, tandasat. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

/// @file
/// @brief Declares interfaces to driver functions.

#ifndef __MEM_ALLOCATOR_DRIVER_H__
#define __MEM_ALLOCATOR_DRIVER_H__

#include "common.h"
#include "..\shared\mem_allocator_shared.h"
#include "allocated_mem_access.h"


extern "C" {


#define MEM_ALLOCATOR_LOGGER(format, ...) \
  DbgPrint("[%ws] ", MEM_ALLOCATOR_NAME); \
  DbgPrint((format), __VA_ARGS__); \
  DbgPrint("\r\n");

////////////////////////////////////////////////////////////////////////////////
//
// macro utilities
//

////////////////////////////////////////////////////////////////////////////////
//
// constants and macros
//

////////////////////////////////////////////////////////////////////////////////
//
// types
//

////////////////////////////////////////////////////////////////////////////////
//
// prototypes
//

////////////////////////////////////////////////////////////////////////////////
//
// variables
//

////////////////////////////////////////////////////////////////////////////////
//
// implementations
//

}  // extern "C"

#endif  // __MEM_ALLOCATOR_DRIVER_H__
