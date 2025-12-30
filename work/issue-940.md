# GitHub Issue #940: HLSLParser Expression Parsing Bug

## Title
`[DEV BUG] HLSLParser: Expression in declaration causes parsing error`

## Issue Details

**Status:** Open
**Type:** Bug
**Milestone:** 4.2
**Author:** [kblaschke](https://github.com/kblaschke)
**Created:** December 9, 2025

## Description

The HLSLParser class encounters a parsing failure when processing variable declarations that include complex expressions on the right-hand side.

### Problem Example

The parser rejects valid HLSL code like:

```hlsl
float scalar = 2.0;
float2 var = (float2(x, y)) * scalar;
```

The parser throws an error at the `*` operator, expecting a semicolon instead. The equivalent GLSL translation is valid:

```glsl
float scalar = 2.0;
vec2 var = (vec2(x, y)) * scalar;
```

### Root Cause

According to the issue, the failure occurs in the parser's end-of-statement validation. The underlying problem likely exists in the `ParseDeclaration()` function, which "doesn't expect additional code besides a simple assignment."

### Required Fix

The parser needs enhancement to recognize and properly handle "any additional expressions which might be within the declaration's rvalue."

## Metadata
- **Project:** libprojectM (including the playlist library)
- **Affected Platforms:** All architectures
- **Labels:** bug
