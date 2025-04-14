import os
import subprocess

def write_source_file(code, filename="test.arc"):
    with open(filename, "w") as f:
        f.write(code)

def compile_and_run():
    subprocess.run(["python3", "ArcCompilerVm.py", "arcbuild", "test.arc"], check=True)
    result = subprocess.run(["python3", "ArcCompilerVm.py", "arcrun", "main.arx"], capture_output=True, text=True)
    return result.stdout.strip()

def run_test_case(name, code, expected_output):
    print(f"\n=== Running {name} ===")
    write_source_file(code)
    try:
        output = compile_and_run()
        assert expected_output in output, f"❌ {name} failed!\nExpected: {expected_output}\nGot: {output}"
        print(f"✅ {name} passed.")
    except Exception as e:
        print(str(e))

# --- TEST CASES ---

# Test 1: Basic Arithmetic
run_test_case(
    "Test_Addition",
    """
    let a = 5
    let b = 7
    c = a + b
    return c
    """,
    "Return: 12"
)

# Test 2: Subtraction
run_test_case(
    "Test_Subtraction",
    """
    let a = 10
    let b = 3
    c = a - b
    return c
    """,
    "Return: 7"
)

# Test 3: While Loop
run_test_case(
    "Test_While_Loop",
    """
    let a = 0
    let b = 5
    while a < b {
        a = a + 1
    }
    return a
    """,
    "Return: 5"
)

# Test 4: If Condition True
run_test_case(
    "Test_If_Condition_True",
    """
    let x = 9
    if x > 5 {
        x = x + 2
    }
    return x
    """,
    "Return: 11"
)

# Test 5: If Condition False
run_test_case(
    "Test_If_Condition_False",
    """
    let x = 2
    if x > 5 {
        x = x + 2
    }
    return x
    """,
    "Return: 2"
)

# Test 6: Equality Check
run_test_case(
    "Test_Equality",
    """
    let a = 4
    let b = 4
    let result = a == b
    return result
    """,
    "Return: true"
)

# Test 7: Inequality Check
run_test_case(
    "Test_Inequality",
    """
    let x = 5
    let y = 3
    let diff = x != y
    return diff
    """,
    "Return: true"
)

# Test 8: Boolean Negation
run_test_case(
    "Test_Boolean_Negation",
    """
    let a = true
    let b = !a
    return b
    """,
    "Return: false"
)

# Test 9: Nested If
run_test_case(
    "Test_Nested_If",
    """
    let a = 10
    if a > 5 {
        if a < 15 {
            a = a + 5
        }
    }
    return a
    """,
    "Return: 15"
)

# Test 10: While Loop with Break Condition
run_test_case(
    "Test_While_With_Condition",
    """
    let i = 0
    while i < 10 {
        if i == 4 {
            break
        }
        i = i + 1
    }
    return i
    """,
    "Return: 4"
)

# Test 11: Chain Assignment
run_test_case(
    "Test_Chain_Assignment",
    """
    let a = 1
    let b = a
    let c = b
    return c
    """,
    "Return: 1"
)

# Test 12: Multiple Returns (ensure last return dominates)
run_test_case(
    "Test_Multiple_Returns",
    """
    let a = 3
    return a
    return 99
    """,
    "Return: 3"
)

# Test 13: Division
run_test_case(
    "Test_Division",
    """
    let x = 20
    let y = 4
    let result = x / y
    return result
    """,
    "Return: 5"
)

# Test 14: Multiplication
run_test_case(
    "Test_Multiplication",
    """
    let x = 6
    let y = 7
    return x * y
    """,
    "Return: 42"
)

# Test 15: Complex Expression
run_test_case(
    "Test_Complex_Expression",
    """
    let x = 10
    let y = 5
    let z = 2
    let r = (x + y) * z - 5
    return r
    """,
    "Return: 25"
)
