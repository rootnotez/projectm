# Issue #940 - Original Code Logic Trace

## Test Case
Parsing: `float2 var = (float2(1.0, 2.0)) * 2.0;`

## Token Stream
```
Position: 0    1      2   3   4       5   6    7    8    9  10  11    12   13
Tokens:  float2 var   =   (   float2  (  1.0  ,   2.0  )   )   *   2.0   ;
```

---

## Original Buggy Code (Lines 2208-2305)

```cpp
2208  bool HLSLParser::ParseBinaryExpression(int priority, HLSLExpression*& expression)
2209  {
2210      const char* fileName = GetFileName();
2211      int         line     = GetLineNumber();
2212
2213      char needsExpressionEndChar;
2214
2215      if (!ParseTerminalExpression(expression, needsExpressionEndChar))
2216      {
2217          return false;
2218      }
2219
2220      // reset priority cause openned parenthesis
2221      if( needsExpressionEndChar != 0 )
2222          priority = 0;
2223
2224      bool acceptBinaryOp = false;
2225      HLSLBinaryOp binaryOp;
2226      while (1)
2227      {
2228          if (acceptBinaryOp || AcceptBinaryOperator(priority, binaryOp))
2229          {
2230              acceptBinaryOp = false;
2231              HLSLExpression* expression2 = NULL;
2232              ASSERT( binaryOp < sizeof(_binaryOpPriority) / sizeof(int) );
2233              if (!ParseBinaryExpression(_binaryOpPriority[binaryOp], expression2))
2234              {
2235                  return false;
2236              }
2237              HLSLBinaryExpression* binaryExpression = m_tree->AddNode<HLSLBinaryExpression>(fileName, line);
2238              binaryExpression->binaryOp    = binaryOp;
2239              binaryExpression->expression1 = expression;
2240              binaryExpression->expression2 = expression2;
2241              // ... type checking code omitted ...
2252
2253              expression = binaryExpression;
2254          }
2255          else if (_conditionalOpPriority > priority && Accept('?'))
2256          {
2257              // ... ternary operator code omitted ...
2282              expression = conditionalExpression;
2283          }
2284          else
2285          {
2286              break;  // ← EXIT POINT: No more operators found
2287          }
2288
2289          //  First: try if next is a binary op before exiting the loop
2290          if (AcceptBinaryOperator(priority, binaryOp))
2291          {
2292              acceptBinaryOp = true;
2293              continue;
2294          }
2295
2296          if( needsExpressionEndChar != 0 )
2297          {
2298              if( !Expect(needsExpressionEndChar) )
2299                  return false;
2300              needsExpressionEndChar = 0;
2301          }
2302      }
2303
2304      return needsExpressionEndChar == 0 || Expect(needsExpressionEndChar);
2305  }
```

---

## Step-by-Step Execution Trace

### Call 1: ParseBinaryExpression(priority=0) from ParseDeclarationAssignment

**Initial State:**
```
Token Position: 3 (pointing at '(')
priority: 0
expression: NULL
needsExpressionEndChar: undefined
```

---

#### Step 1: ParseTerminalExpression (Line 2215)

**Enters ParseTerminalExpression at line 2326:**

```
Token Position: 3
Current Token: '('
```

**Substeps within ParseTerminalExpression:**

1.1. **Line 2331:** `needsExpressionEndChar = 0`

1.2. **Line 2406:** `Accept('(')` → **TRUE** (consumes token)
```
Token Position: 4 (now at 'float2')
needsExpressionEndChar: still 0
expressionEndChar: ')'
```

1.3. **Line 2420:** `AcceptType(false, type)` → **TRUE** (accepts 'float2')
```
Token Position: 5 (now at '(')
type.baseType: HLSLBaseType_Float2
```

1.4. **Line 2423:** `Accept('(')` → **TRUE** (consumes token)
```
Token Position: 6 (now at '1.0')
```

1.5. **Line 2425:** `needsExpressionEndChar = expressionEndChar` → `needsExpressionEndChar = ')'`
```
needsExpressionEndChar: ')'  ← CRITICAL: We need to match the OUTER paren
```

1.6. **Line 2426:** Calls `ParsePartialConstructor(expression, float2, "float2")`

   **Inside ParsePartialConstructor:**
   - Line 2316: `ParseExpressionList(')', ...)`
     - Parses: `1.0 , 2.0`
     - Expects and **consumes** the closing `)`
   ```
   Token Position: 9 (now at ')')  ← INNER closing paren consumed
   ```
   - Returns successfully with constructor expression

1.7. **Returns from ParseTerminalExpression**
```
Token Position: 9 (at OUTER ')')
expression: ConstructorExpression(float2(1.0, 2.0))
needsExpressionEndChar: ')'  ← Still set! Expecting outer paren
```

---

#### Step 2: Priority Reset (Lines 2221-2222)

```
Line 2221: needsExpressionEndChar != 0 → TRUE
Line 2222: priority = 0
```

**State:**
```
Token Position: 9 (at ')')
priority: 0 (reset because we're in parens)
needsExpressionEndChar: ')'
expression: ConstructorExpression
```

---

#### Step 3: Enter While Loop (Line 2226)

**State:**
```
acceptBinaryOp: false
binaryOp: undefined
```

---

#### Step 4: Line 2228 - Try to Accept Binary Operator

```
Line 2228: acceptBinaryOp = false, so evaluate AcceptBinaryOperator(0, binaryOp)
```

**Inside AcceptBinaryOperator (priority=0):**
- Current token: `')'`
- This is NOT a binary operator
- Returns **FALSE**

**Result:**
```
Condition (acceptBinaryOp || AcceptBinaryOperator(...)) → (false || false) → FALSE
Skip lines 2229-2254
```

---

#### Step 5: Line 2256 - Try Ternary Operator

```
Line 2256: _conditionalOpPriority > 0 && Accept('?')
```

- Current token: `')'`
- This is NOT `'?'`
- Returns **FALSE**

**Result:**
```
Skip lines 2257-2283
```

---

#### Step 6: Line 2284-2287 - ELSE BLOCK - THE BUG LOCATION

```
Line 2284: else
Line 2286: break;  ← EXIT THE WHILE LOOP
```

**Critical Point:**
- We break from the loop WITHOUT executing lines 2289-2301
- Lines 2289-2301 are AFTER the closing brace of the else block
- Those lines would have consumed the `')'` token

**State After Break:**
```
Token Position: 9 (STILL at ')')
needsExpressionEndChar: ')' (STILL SET!)
expression: ConstructorExpression
```

---

#### Step 7: Line 2304 - Final Return

```
Line 2304: return needsExpressionEndChar == 0 || Expect(needsExpressionEndChar);
```

**Evaluation:**
```
needsExpressionEndChar == 0  → ')' == 0  → FALSE
|| Expect(needsExpressionEndChar)  → Expect(')')
```

**Inside Expect(')'):**
- Current token at position 9: `')'`
- **Consumes the token**
```
Token Position: 10 (now at '*')
```
- Returns **TRUE**

**ParseBinaryExpression returns TRUE**

---

### Back to ParseDeclarationAssignment (Line 2012)

```
ParseExpression returned TRUE
Token Position: 10 (at '*')
```

---

### Back to ParseDeclaration (Line 1977-1985)

After line 1979, expecting to find more declarations or statement end:

```
Line 1985: while(Accept(','))  → Current token is '*', not ','  → FALSE
```

**Exit the declaration parsing loop**

---

### ParseStatement (calling ParseDeclaration)

After ParseDeclaration returns, ParseStatement expects a semicolon:

```
Current Token: '*' (position 10)
Expected: ';'
```

**ERROR: "expected ';' near '*'"**

---

## Root Cause Analysis

### The Problem

When the parser encounters the pattern `(constructor) * expr`:

1. **ParseTerminalExpression** sets `needsExpressionEndChar = ')'` for the outer paren
2. After parsing the constructor, the token stream is at the **closing `)`**
3. The while loop at line 2226 tries to find a binary operator or ternary
4. Neither is found (current token is `')'`, not an operator)
5. **BREAKS at line 2286**
6. Lines 2289-2301 are **NEVER EXECUTED** because they're after the else block
7. The `')'` is only consumed at line 2304 during the return
8. After consuming `')'`, the function returns immediately
9. The `'*'` operator that follows is **NEVER PARSED**

### Why Lines 2289-2301 Don't Execute

```
while (1) {
    if (found_binary_op) {
        // ... process it ...
    } else if (found_ternary) {
        // ... process it ...
    } else {
        break;  ← EXIT HERE
    }

    // Lines 2289-2301 are HERE
    // They only execute AFTER processing an operator
    // NOT after breaking from the else block
}
```

### The Loop Logic Flaw

Lines 2289-2301 were intended to:
1. Check for another operator after processing one
2. Consume end character if needed

But they only execute after **successfully** processing a binary/ternary operator, not after the initial terminal expression.

---

## Visual Flow Diagram

```
ParseBinaryExpression(priority=0)
│
├─ ParseTerminalExpression
│  └─ Parses: (float2(1.0, 2.0))
│     Returns: needsExpressionEndChar=')'
│     Token at: position 9 (')')
│
├─ Reset priority to 0
│
├─ Enter while loop
│  │
│  ├─ Try AcceptBinaryOperator(0, ...)
│  │  Current token: ')'
│  │  Result: FALSE (not an operator)
│  │
│  ├─ Try ternary '?'
│  │  Current token: ')'
│  │  Result: FALSE
│  │
│  └─ else: BREAK ← BUG: Exit without consuming ')'
│     Token still at: position 9 (')')
│     [Lines 2289-2301 SKIPPED]
│
└─ Return (line 2304)
   ├─ needsExpressionEndChar == 0? NO (it's ')')
   ├─ Expect(')')
   │  └─ Consumes ')' at position 9
   │     Token now at: position 10 ('*')
   └─ Return TRUE

Back in ParseDeclaration
│  Current token: '*'
│  Expected: ';' or ','
│
└─ ERROR: expected ';' near '*'
```

---

## Key Variables State Table

| Step | Line | Token Pos | Token | needsEndChar | priority | Action |
|------|------|-----------|-------|--------------|----------|--------|
| 1 | 2215 | 3 | `(` | undefined | 0 | Call ParseTerminalExpression |
| 2 | 2331 | 3 | `(` | 0 | 0 | Inside ParseTerminalExpression |
| 3 | 2406 | 3→4 | `(` | 0 | 0 | Accept '(', set expressionEndChar=')' |
| 4 | 2420 | 4→5 | `float2` | 0 | 0 | Accept type |
| 5 | 2423 | 5→6 | `(` | 0 | 0 | Accept '(' for constructor |
| 6 | 2425 | 6 | `1.0` | **')'** | 0 | Set needsEndChar to ')' |
| 7 | 2316 | 6→9 | varies | ')' | 0 | Parse constructor args, consume inner ')' |
| 8 | 2222 | 9 | `)` | ')' | **0** | Reset priority |
| 9 | 2228 | 9 | `)` | ')' | 0 | Try AcceptBinaryOp → FALSE |
| 10 | 2286 | 9 | `)` | ')' | 0 | **BREAK** from loop |
| 11 | 2304 | 9→10 | `)` | ')' | 0 | Expect(')'), consume it |
| 12 | 2304 | 10 | `*` | 0 | 0 | Return TRUE |
| ERROR | - | 10 | `*` | - | - | Expected ';', found '*' |

---

## Why The Fix Works

The fix moves the end-character consumption into the else block BEFORE breaking:

```cpp
else {
    if (needsExpressionEndChar != 0) {
        Expect(needsExpressionEndChar);  // Consume ')'
        needsExpressionEndChar = 0;

        // NOW check for operators
        if (AcceptBinaryOperator(priority, binaryOp)) {
            acceptBinaryOp = true;
            continue;  // Continue loop to process '*'
        }
    }
    break;
}
```

With this fix:
1. Parse `(float2(1.0, 2.0))` → needsEndChar=')'
2. Try to find operator → NO (token is ')')
3. Execute else block:
   - Consume ')' → token now at '*'
   - Check for operator → YES! Found '*'
   - Set acceptBinaryOp=true and continue
4. Loop iteration 2: Process the '*' operator
5. Parse right side: `2.0`
6. Build binary expression: `(float2(1.0, 2.0)) * 2.0`
7. Return successfully

Token stream properly advances and the multiplication is parsed!
