// Copyright (C) 2024 owoDra

#pragma once

#include "Logging/LogMacros.h"

GAHADDON_API DECLARE_LOG_CATEGORY_EXTERN(LogGAHA, Log, All);

#if !UE_BUILD_SHIPPING

#define GAHALOG(FormattedText, ...) UE_LOG(LogGAHA, Log, FormattedText, __VA_ARGS__)

#define GAHAENSURE(InExpression) ensure(InExpression)
#define GAHAENSURE_MSG(InExpression, InFormat, ...) ensureMsgf(InExpression, InFormat, __VA_ARGS__)
#define GAHAENSURE_ALWAYS_MSG(InExpression, InFormat, ...) ensureAlwaysMsgf(InExpression, InFormat, __VA_ARGS__)

#define GAHACHECK(InExpression) check(InExpression)
#define GAHACHECK_MSG(InExpression, InFormat, ...) checkf(InExpression, InFormat, __VA_ARGS__)

#else

#define GAHALOG(FormattedText, ...)

#define GAHAENSURE(InExpression) InExpression
#define GAHAENSURE_MSG(InExpression, InFormat, ...) InExpression
#define GAHAENSURE_ALWAYS_MSG(InExpression, InFormat, ...) InExpression

#define GAHACHECK(InExpression) InExpression
#define GAHACHECK_MSG(InExpression, InFormat, ...) InExpression

#endif