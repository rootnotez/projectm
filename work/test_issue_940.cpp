#include "HLSLParser.h"
#include "HLSLTree.h"
#include <iostream>
#include <cstring>

bool testParse(const char* testName, const char* hlslCode) {
    M4::Allocator allocator;
    M4::HLSLTree tree(&allocator);
    M4::HLSLParser parser(&allocator, &tree);

    bool result = parser.Parse("test.hlsl", hlslCode, strlen(hlslCode));

    std::cout << "[" << (result ? "PASS" : "FAIL") << "] " << testName << std::endl;
    if (!result) {
        std::cout << "  Code: " << hlslCode << std::endl;
    }

    return result;
}

int main() {
    std::cout << "Testing HLSLParser Issue #940: Complex Expression Parsing\n";
    std::cout << "==========================================================\n\n";

    int passed = 0;
    int total = 0;

    // Test 1: Original failing case with variable reference
    total++;
    std::cout << "Test 1: Original failing case (constructor with variable multiplication)...\n";
    if (testParse("Complex constructor expression",
                  "float scalar = 2.0;\n"
                  "float2 var = (float2(1.0, 2.0)) * scalar;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 2: Simpler version with literal
    total++;
    std::cout << "Test 2: Constructor with literal multiplication...\n";
    if (testParse("Constructor with multiplication",
                  "float2 var = (float2(1.0, 2.0)) * 2.0;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 3: Without outer parens (baseline - should work)
    total++;
    std::cout << "Test 3: Constructor without outer parens (baseline)...\n";
    if (testParse("Constructor without parens",
                  "float2 var = float2(1.0, 2.0) * 2.0;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 4: Nested parens with addition
    total++;
    std::cout << "Test 4: Nested parens with addition operator...\n";
    if (testParse("Nested parens with addition",
                  "float2 var = (float2(1.0, 2.0)) + float2(3.0, 4.0);\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 5: Multiple operations
    total++;
    std::cout << "Test 5: Multiple operations in sequence...\n";
    if (testParse("Multiple operations",
                  "float3 var = (float3(1.0, 2.0, 3.0)) * 2.0 + float3(0.5, 0.5, 0.5);\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 6: Double nested parens
    total++;
    std::cout << "Test 6: Double nested parentheses...\n";
    if (testParse("Double nested parens",
                  "float2 var = ((float2(1.0, 2.0))) * 2.0;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 7: Constructor in right operand
    total++;
    std::cout << "Test 7: Constructor as right operand...\n";
    if (testParse("Constructor in right operand",
                  "float2 var = 2.0 * (float2(1.0, 2.0));\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 8: Mixed constructors
    total++;
    std::cout << "Test 8: Both operands are constructors...\n";
    if (testParse("Mixed constructors",
                  "float2 var = (float2(1.0, 2.0)) * (float2(3.0, 4.0));\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 9: Division operator
    total++;
    std::cout << "Test 9: Constructor with division...\n";
    if (testParse("Constructor with division",
                  "float2 var = (float2(4.0, 8.0)) / 2.0;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 10: Subtraction operator
    total++;
    std::cout << "Test 10: Constructor with subtraction...\n";
    if (testParse("Constructor with subtraction",
                  "float2 var = (float2(5.0, 6.0)) - float2(1.0, 2.0);\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 11: Ternary operator with parentheses
    total++;
    std::cout << "Test 11: Ternary operator with parentheses...\n";
    if (testParse("Ternary with parens",
                  "bool cond = true; float x = (cond) ? 1.0 : 2.0;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 12: Double-nested parentheses with operator
    total++;
    std::cout << "Test 12: Double-nested parentheses with operator...\n";
    if (testParse("Double-nested parens with op",
                  "float a = 1.0; float b = 2.0; float x = ((a + b)) * 3.0;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 13: Constructor in ternary expression
    total++;
    std::cout << "Test 13: Constructor in ternary expression...\n";
    if (testParse("Constructor in ternary",
                  "bool cond = true; float s = 2.0; float2 x = cond ? (float2(1.0, 2.0)) * s : float2(0.0, 0.0);\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 14: Chained operators after parentheses
    total++;
    std::cout << "Test 14: Chained operators after parentheses...\n";
    if (testParse("Chained operators",
                  "float a = 1.0; float b = 2.0; float c = 3.0; float x = (a) * b + c;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 15: Triple-nested parentheses
    total++;
    std::cout << "Test 15: Triple-nested parentheses...\n";
    if (testParse("Triple-nested parens",
                  "float a = 1.0; float x = (((a))) * 2.0;\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Test 16: Multiple parenthesized expressions
    total++;
    std::cout << "Test 16: Multiple parenthesized expressions...\n";
    if (testParse("Multiple paren expressions",
                  "float2 x = (float2(1.0, 2.0)) * (float2(3.0, 4.0)) + (float2(5.0, 6.0));\n")) {
        passed++;
    }
    std::cout << std::endl;

    // Summary
    std::cout << "==========================================================\n";
    std::cout << "Results: " << passed << "/" << total << " tests passed\n";

    if (passed == total) {
        std::cout << "\n✓ All tests PASSED!\n";
        return 0;
    } else {
        std::cout << "\n✗ " << (total - passed) << " test(s) FAILED\n";
        return 1;
    }
}
