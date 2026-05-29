//  Error: String inequality check
// Ensure that the compiler rejects comparison between incompatible types even
// in inequality.
// TEST_F(JITOutputTest, Error_InequalityStringBool) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let a string := "hello";
//             let b bool := true;
//             if (a != b) { return 1; }
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_AndBoolInt) {
//  std::string input = "let a bool := true && 1;";
//  std::string expected_diagnostic = "";
//  assertSwaOutput(input, expected_diagnostic);
//}
// FIXME
// TEST_F(JITOutputTest, Error_CompareIntString) {
//   std::string input = "let a bool := (10 + 20) < \"30\";";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_ComplexLogicMismatch) {
//   std::string input = "let x bool := (10 < 20) + (true && false);";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_StringModuloInt) {
//   std::string input = "let x int := \"abc\" % 3;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_OrStringBool) {
//   std::string input = "let x bool := \"true\" || true;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_GreaterEqualStringInt) {
//   std::string input = "let x bool := \"10\" >= 10;";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_NotString) {
//   std::string input = "let x bool := !\"hello\";";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_MixedComparisonChain) {
//   std::string input = "let x bool := 10 == \"10\";";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

//  Invalid Identity/Self-Assignment
// Ensuring the compiler prevents operating on an uninitialized or wrong-type
// self-reference
// TEST_F(JITOutputTest, Error_CircularTypeDependency) {
//   std::string input = "let x int := x + 1;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_UndefinedVariable) {
//   std::string input = "let a int := b + 10;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// Tests the boundary between the SymbolTable declaration phase and
// the type-checking phase for re-assignment.
// TEST_F(JITOutputTest, Program_Boundary_SelfAssignment) {
//   std::string input = R"(
// dialect:english;
// start() int {
//     let x int := 10;
//     x := x + 10;
//     return 0;
// }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Program_Valid_CallWithDefinitions) {
//   std::string input = R"(
//         dialect:english;
//
//         add(a int, b int) int {
//             return a + b;
//         }
//
//         start() int {
//             let res int := add(10, 20);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// TEST_F(JITOutputTest, Program_Error_CallWithMissingDefinition) {
//   std::string input = R"(
//         dialect:english;
//
//         start() int {
//             let res int := unknown_function(10);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// TEST_F(JITOutputTest, Error_CallArgMismatchInDefinition) {
//   std::string input = R"(
//         dialect:english;
//
//         multiply(a int, b int) int { return a * b; }
//
//         start() int {
//             // Error: multiply expects 2 arguments, only 1 provided
//             let res int := multiply(5);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// TEST_F(JITOutputTest, Error_CallTypeMismatchInDefinition) {
//   std::string input = R"(
//         dialect:english;
//
//         show(msg string) int { return 0; }
//
//         start() int {
//             // Error: show expects string, got int
//             let res int := show(100);
//             return 0;
//         }
//     )";
////   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// //  Error: Argument count mismatch (too few)
// TEST_F(JITOutputTest, Error_Call_TooFewArguments) {
//   std::string input = R"(
//         dialect:english;
//         func calculate(a int, b int) int { return a + b; }
//         start() int {
//             let res int := calculate(10);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// //  Error: Argument count mismatch (too many)
// TEST_F(JITOutputTest, Error_Call_TooManyArguments) {
//   std::string input = R"(
//         dialect:english;
//         func calculate(a int, b int) int { return a + b; }
//         start() int {
//             let res int := calculate(10, 20, 30);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Argument count mismatch for 'calculate'");
// }
//
// //  Error: Argument type mismatch
// TEST_F(JITOutputTest, Error_Call_ArgumentTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func log(msg string) int { return 0; }
//         start() int {
//             let res int := log(42);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Type mismatch in argument for 'log'");
// }
//
// //  Error: Type mismatch in nested function call
// TEST_F(JITOutputTest, Error_Call_NestedArgumentMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func get_name() string { return "test"; }
//         func process(val int) int { return val; }
//         start() int {
//             let res int := process(get_name());
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Type mismatch in argument for 'process'");
// }
//
// //  Error: Function return type mismatch in assignment
// TEST_F(JITOutputTest, Error_Call_AssignmentTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func get_val() int { return 10; }
//         start() int {
//             let res string := get_val();
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot assign Int to variable of type
//   String");
// }
//
// // 7. Error: Calling a function with an invalid expression as an argument
// TEST_F(JITOutputTest, Error_Call_InvalidExpressionArgument) {
//   std::string input = R"(
//         dialect:english;
//         func process(val int) int { return val; }
//         start() int {
//             let res int := process("invalid" + 10);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on String and Int");
// }
// //  Error: Function call on a non-callable identifier
// // Prevents treating a variable as a function (common in languages with
// // first-class functions)
// TEST_F(JITOutputTest, Error_Call_IdentifierIsNotFunction) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let x int := 10;
//             let y int := x(5); // Error: x is a variable, not a function
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Identifier 'x' is not a function");
// }
//
// //  Error: Shadowing an existing function with a variable
// // Ensures the compiler detects name collisions between scopes and function
// // tables
// TEST_F(JITOutputTest, Error_Call_ShadowedFunction) {
//   std::string input = R"(
//         dialect:english;
//         func foo() int { return 1; }
//         start() int {
//             let foo int := 10;
//             return foo(5); // Error: local 'foo' shadows function 'foo'
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Identifier 'foo' is a variable, not a function");
// }
//
// //  Error: Recursive call with incorrect return type propagation
// // Validates that the return type is checked against the *declaration*, not
// the
// // incomplete body
// TEST_F(JITOutputTest, Error_Call_RecursiveReturnTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func recursive(n int) int {
//             return recursive(n) + "wrong"; // Error: recursive returns int,
//             not string
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on Int and String");
// }
//
// //  Error: Argument expression with side effects/type drift
// // Verifies that type inference doesn't drift when multiple operations are
// // involved
// TEST_F(JITOutputTest, Error_Call_ComplexTypeDrift) {
//   std::string input = R"(
//         dialect:english;
//         func compute(a int) int { return a; }
//         start() int {
//             // Error: (10 + "20") is a type error, regardless of the function
//             call let res int := compute(10 + "20"); return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on Int and String");
// }
//
// //  Error: Attempting to call a function defined in a deeper scope
// // Validates that lexical scoping prevents calling functions defined inside
// // other blocks
// TEST_F(JITOutputTest, Error_Call_ScopeVisibility) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             {
//                 func hidden() int { return 1; }
//             }
//             return hidden(); // Error: 'hidden' is not in the global/parent
//             scope
//         }
//     )";
//   assertSwaOutput(input, "Error: Function 'hidden' is undefined");
// }
//
// //  Error: Recursive depth/Invalid base case logic
// // Verifies that the compiler enforces return type consistency across all
// // branches
// TEST_F(JITOutputTest, Error_Call_InconsistentReturnTypes) {
//   std::string input = R"(
//         dialect:english;
//         func test(a int) int {
//             if (a == 0) { return "oops"; } // Error: returns string, expected
//             int return a;
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Function 'test' returns String, expected
//   Int");
// }
//
//  Error: Identifier name collision between parameter and function
// // Validates that parameter names do not accidentally "hijack" function
// // resolution.
// TEST_F(JITOutputTest, Error_Call_ParameterShadowsFunction) {
//   std::string input = R"(
//         dialect:english;
//         func calculate(a int) int { return a; }
//         func test(calculate int) int {
//             // 'calculate' here refers to the parameter, not the function.
//             // Attempting to invoke the parameter as a function should error.
//             return calculate(10);
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(
//       input, "Error: Identifier 'calculate' is a variable, not a function");
// }
//
// //  Error: Void-like function return value assignment
// // If your language supports functions that don't return values (or you plan
// // to), ensure you cannot assign a 'void' result to a typed variable.
// TEST_F(JITOutputTest, Error_Call_AssignVoidToVariable) {
//   std::string input = R"(
//         dialect:english;
//         func procedure() { /* No return type */ }
//         start() int {
//             let x int := procedure();
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Function 'procedure' does not return a
//   value");
// }
//
// //  Error: Circular dependency in type inference
// // This checks for the "infinite loop" potential when an argument's type
// depends
// // on the function call itself, which in turn depends on the argument.
// TEST_F(JITOutputTest, Error_Call_CircularDependency) {
//   std::string input = R"(
//         dialect:english;
//         func identity(a int) int { return a; }
//         start() int {
//             let x int := identity(x + 1);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Recursive dependency in type inference for 'x'");
// }
//
// //  Error: Calling a function within its own default parameter (if you
// expand
// // syntax) Or simply passing an uninitialized variable into a function.
// TEST_F(JITOutputTest, Error_Call_UninitializedArgument) {
//   std::string input = R"(
//         dialect:english;
//         func process(a int) int { return a; }
//         start() int {
//             let x int; // Uninitialized
//             return process(x);
//         }
//     )";
//   assertSwaOutput(input, "Error: Variable 'x' used before initialization");
// }
//
// //  Error: Expression result as function name
// // Modern parsers should prevent an expression (like a variable or logic)
// from
// // being evaluated as a function pointer unless the language explicitly
// supports
// // it.
// TEST_F(JITOutputTest, Error_Call_ExpressionAsCallee) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let op string := "add";
//             return op(10, 20); // Error: op is a string expression, not a
//             function identifier
//         }
//     )";
//   assertSwaOutput(input, "Error: 'op' is not a callable function");
// }
//
// //  Error: Argument count mismatch with variadic-like signatures
// // If your language doesn't support them, ensure it explicitly rejects
// trailing
// // commas or extra arguments that mimic variadic calls.
// TEST_F(JITOutputTest, Error_Call_TrailingCommaMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func add(a int, b int) int { return a + b; }
//         start() int {
//             // Parsing error or semantic error for dangling argument/trailing
//             comma return add(10, 20, );
//         }
//     )";
//   assertSwaOutput(input, "Error: Unexpected token in argument list");
// }
//
//  Error: Function call on a primitive literal
// // Prevents treating a literal value as a function, similar to GCC's
// diagnostic
// // when someone writes 10(5) instead of 10 * 5.
// TEST_F(JITOutputTest, Error_Call_PrimitiveLiteralAsFunction) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let x int := 10(5); // Error: 10 is an Int, not a function
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: '10' is not a function");
// }
//
// //  Error: Argument expression is a function that returns void
// // Ensures that you cannot use the return value of a void function in an
// // argument expression.
// TEST_F(JITOutputTest, Error_Call_PassingVoidResult) {
//   std::string input = R"(
//         dialect:english;
//         func do_nothing() { }
//         func take_int(a int) int { return a; }
//         start() int {
//             let x int := take_int(do_nothing()); // Error: do_nothing returns
//             nothing return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot pass void result to function");
// }
//
// //  Error: Function call name collision with built-in or keyword (if
// // applicable) Tests if your scope resolution correctly prioritizes
// user-defined
// // functions over local variables that share names.
// TEST_F(JITOutputTest, Error_Call_NameCollisionWithLocal) {
//   std::string input = R"(
//         dialect:english;
//         func test() int { return 1; }
//         start() int {
//             let test int := 10;
//             // The compiler must resolve 'test' to the Int variable, not the
//             function return test(5);
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Identifier 'test' is a variable, not a function");
// }
// //  Error: Type mismatch via implicit narrowing (if you support Float/Int)
// // If you eventually support multiple numeric types, ensure you don't allow
// // implicit narrowing if your language policy is strict.
// TEST_F(JITOutputTest, Error_Call_NarrowingConversion) {
//   std::string input = R"(
//         dialect:english;
//         func accept_int(a int) int { return a; }
//         start() int {
//             let x float := 10.5;
//             let y int := accept_int(x); // Error: Implicit float to int
//             narrowing return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot pass Float to function expecting
//   Int");
// }
// //  Error: Recursive call in global scope (if prohibited)
// // Some compilers strictly enforce that global-level expressions must be
// // constants.
// TEST_F(JITOutputTest, Error_Call_IllegalGlobalCall) {
//   std::string input = R"(
//         dialect:english;
//         func foo() int { return 1; }
//         let x int := foo(); // Error: Global initializers must be constant
//         expressions start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Global initializers must be constant");
// }
// // Error: Function call as an L-Value
// // Prevents the compiler from trying to assign a value to the *result* of a
// // function call.
// TEST_F(JITOutputTest, Error_Call_AssignmentToResult) {
//   std::string input = R"(
//         dialect:english;
//         func get_val() int { return 10; }
//         start() int {
//             get_val() := 20; // Error: Cannot assign to function result
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Expression is not assignable");
// }
//
// //  Error: Type mismatch with unary negation inside call
// // Ensures the visitor correctly resolves the unary operator's type before
// // passing it to the function.
// TEST_F(JITOutputTest, Error_Call_NegatedTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func process(val int) int { return val; }
//         start() int {
//             let s string := "test";
//             // Unary '-' is likely invalid for strings; visitor should
//             propagate this error let x int := process(-s); return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Operator '-' not supported for type
//   'String'");
// }
//
// //  Error: Function identifier conflict with keyword
// // Prevents naming functions after internal keywords or reserved symbols.
// TEST_F(JITOutputTest, Error_Call_KeywordAsFunctionName) {
//   std::string input = R"(
//         dialect:english;
//         func if(a int) int { return a; } // Error: 'if' is a reserved keyword
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input,
//                   "Error: Illegal use of reserved keyword 'if' as
//                   identifier");
// }
//
// //  Error: Argument count with potential "default" value ambiguity
// // Tests if your parser/checker incorrectly assumes existence of default
// params. TEST_F(JITOutputTest, Error_Call_ExplicitArityViolation) {
//   std::string input = R"(
//         dialect:english;
//         func add(a int, b int) int { return a + b; }
//         start() int {
//             // Testing explicitly that we do NOT allow implied missing
//             arguments return add(10);
//         }
//     )";
//   assertSwaOutput(input, "Error: Argument count mismatch for 'add'");
// }
//
// //  Error: Call within a condition expression of an `if` block
// // Ensures that the boolean-only requirement for `if` conditions propagates
// // through the function's return type.
// TEST_F(JITOutputTest, Error_Call_InvalidReturnTypeInIf) {
//   std::string input = R"(
//         dialect:english;
//         func get_int() int { return 1; }
//         start() int {
//             // 'if' requires a boolean condition, but get_int() returns an
//             int if (get_int()) { return 0; } return 1;
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Condition expression requires 'Bool', found
//                   'Int'");
// }
//
// //  Error: Recursive call exceeding scope definition
// // Validates that the return type of a recursive function is fixed by the
// // *first* declaration, and mismatches in the body are caught.
// TEST_F(JITOutputTest, Error_Call_RecursiveBodyMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func recursive(n int) int {
//             // Even if 'recursive' is called here, it must match the return
//             type 'int' return recursive(n - 1) + "oops";
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on Int and String");
// }
// //  Error: Argument-Dependent Resolution Failure
// // Tests that your lookup doesn't accidentally pick up a function from a
// child
// // scope when the function is called in the parent scope.
// TEST_F(JITOutputTest, Error_Call_ScopeLookupIsolation) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             {
//                 func local_func() int { return 1; }
//             }
//             // Error: 'local_func' is not visible here
//             return local_func();
//         }
//     )";
//   assertSwaOutput(input, "Error: Function 'local_func' is undefined");
// }
// //  Error: Recursive Call with Mismatched Argument Count
// // Ensures that the recursive call is checked against the prototype, not the
// // call site.
// TEST_F(JITOutputTest, Error_Call_RecursiveArity) {
//   std::string input = R"(
//         dialect:english;
//         func recurse(a int) int {
//             return recurse(a, 10); // Error: expected 1 arg, got 2
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Argument count mismatch for 'recurse'");
// }

// FIXME
// // Explicitly forces the parser to ignore standard precedence, checking if
// the
// // visitor correctly respects the AST node priority.
// TEST_F(JITOutputTest, Program_Precedence_GroupingOverride) {
//   std::string input = R"(
// dialect:english;
// start() int {
//     let result int := (10 + 5) * 2;
//     return 0;
// }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// //  Error: Comparing String and Int
// // A classic error: programmers often forget to parse a string before
// comparing
// // it to an int.
// TEST_F(JITOutputTest, Error_CompareStringAndInt) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let x int := 10;
//             let y string := "10";
//             if (x = y) { return 1; }
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// //  Error: Using comparison operators on Booleans
// // While some languages allow this (false < true), strict compilers often
// // prevent it.
// TEST_F(JITOutputTest, Error_CompareBooleansWithOperators) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let a bool := true;
//             let b bool := false;
//             if (a > b) { return 1; }
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// //  Error: Comparing result of a math expression to a String
// TEST_F(JITOutputTest, Error_CompareMathResultToString) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let x int := 5 + 5;
//             if (x < "10") { return 1; }
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
