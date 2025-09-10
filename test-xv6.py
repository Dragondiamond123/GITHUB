#!/usr/bin/env python3

import argparse
import subprocess
import sys
import time
import re
import os
from pathlib import Path

class XV6Tester:
    def __init__(self, timeout=60, quiet=False):
        self.timeout = timeout
        self.quiet = quiet
        self.qemu_process = None
        
    def log(self, message):
        if not self.quiet:
            print(f"[TEST] {message}")
    
    def start_xv6(self):
        """Start xv6 in QEMU"""
        try:
            self.log("Starting xv6...")
            self.qemu_process = subprocess.Popen(
                ['make', 'qemu-nox'],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=0
            )
            time.sleep(2)  # Give xv6 time to boot
            return True
        except Exception as e:
            self.log(f"Failed to start xv6: {e}")
            return False
    
    def stop_xv6(self):
        """Stop xv6"""
        if self.qemu_process:
            try:
                # Send Ctrl-A X to quit QEMU
                self.qemu_process.stdin.write('\x01x')
                self.qemu_process.stdin.flush()
                self.qemu_process.wait(timeout=5)
            except:
                self.qemu_process.kill()
            self.qemu_process = None
    
    def run_command(self, command, expected_output=None, timeout=10):
        """Run a command in xv6 and check output"""
        if not self.qemu_process:
            return False, "xv6 not running"
        
        try:
            # Send command
            self.qemu_process.stdin.write(command + '\n')
            self.qemu_process.stdin.flush()
            
            # Read output with timeout
            output = ""
            start_time = time.time()
            
            while time.time() - start_time < timeout:
                if self.qemu_process.poll() is not None:
                    return False, "xv6 process died"
                
                try:
                    line = self.qemu_process.stdout.readline()
                    if line:
                        output += line
                        if expected_output and expected_output in output:
                            return True, output
                        if "$ " in line:  # Shell prompt
                            break
                except:
                    continue
            
            if expected_output:
                return expected_output in output, output
            return True, output
            
        except Exception as e:
            return False, str(e)
    
    def test_basic_commands(self):
        """Test basic shell commands"""
        tests = [
            ("ls", "README"),
            ("echo hello", "hello"),
            ("cat README", "xv6 is a re-implementation"),
            ("wc README", None),  # Just check it runs
            ("mkdir testdir", None),
            ("ls", "testdir"),
            ("rm testdir", None),
        ]
        
        passed = 0
        total = len(tests)
        
        for command, expected in tests:
            self.log(f"Running: {command}")
            success, output = self.run_command(command, expected)
            if success:
                self.log(f"✓ PASS: {command}")
                passed += 1
            else:
                self.log(f"✗ FAIL: {command}")
                if not self.quiet:
                    print(f"    Output: {output[:200]}...")
        
        return passed, total
    
    def test_system_calls(self):
        """Test system calls using usertests"""
        self.log("Running system call tests...")
        success, output = self.run_command("usertests -q", "ALL TESTS PASSED", 120)
        
        if success:
            self.log("✓ PASS: System call tests")
            return 1, 1
        else:
            self.log("✗ FAIL: System call tests")
            if not self.quiet:
                print(f"    Output: {output[-500:]}")  # Show last part of output
            return 0, 1
    
    def test_user_programs(self):
        """Test user programs"""
        programs = [
            "forktest",
            "echo test args",
            "grep the README",
            "kill 1",  # This should fail gracefully
            "ln README testlink",
        ]
        
        passed = 0
        total = len(programs)
        
        for program in programs:
            self.log(f"Testing: {program}")
            success, output = self.run_command(program)
            if success or "cannot" in output.lower():  # Some failures are expected
                self.log(f"✓ PASS: {program}")
                passed += 1
            else:
                self.log(f"✗ FAIL: {program}")
        
        return passed, total
    
    def run_specific_test(self, test_pattern):
        """Run tests matching a specific pattern"""
        if not self.start_xv6():
            return False
        
        try:
            total_passed = 0
            total_tests = 0
            
            if re.search(r'basic|command', test_pattern, re.I):
                passed, total = self.test_basic_commands()
                total_passed += passed
                total_tests += total
            
            if re.search(r'system|syscall|usertests', test_pattern, re.I):
                passed, total = self.test_system_calls()
                total_passed += passed
                total_tests += total
            
            if re.search(r'user|program', test_pattern, re.I):
                passed, total = self.test_user_programs()
                total_passed += passed
                total_tests += total
            
            if re.search(r'all|.*', test_pattern, re.I) and test_pattern != "all":
                # If pattern doesn't match specific categories, run all
                if total_tests == 0:
                    passed, total = self.test_basic_commands()
                    total_passed += passed
                    total_tests += total
                    
                    passed, total = self.test_system_calls()
                    total_passed += passed
                    total_tests += total
                    
                    passed, total = self.test_user_programs()
                    total_passed += passed
                    total_tests += total
            
            self.log(f"Results: {total_passed}/{total_tests} tests passed")
            return total_passed == total_tests
            
        finally:
            self.stop_xv6()

def main():
    parser = argparse.ArgumentParser(
        description='Test runner for xv6 operating system',
        epilog='''
Examples:
  python test-xv6.py basic          # Run basic command tests
  python test-xv6.py syscall        # Run system call tests
  python test-xv6.py user           # Run user program tests
  python test-xv6.py all            # Run all tests
  python test-xv6.py -q system      # Run system tests quietly
        '''
    )
    
    parser.add_argument('testrex', 
                       help='Test pattern to run (basic, syscall, user, all, or regex)')
    parser.add_argument('-q', '--quiet', 
                       action='store_true',
                       help='Run quietly with minimal output')
    parser.add_argument('-t', '--timeout',
                       type=int, default=60,
                       help='Timeout for tests in seconds (default: 60)')
    
    args = parser.parse_args()
    
    # Check if we're in the right directory
    if not Path('Makefile').exists() or not Path('kernel').exists():
        print("Error: Please run this script from the xv6-riscv directory")
        sys.exit(1)
    
    # Build xv6 first
    print("Building xv6...")
    try:
        result = subprocess.run(['make', 'clean'], capture_output=True, text=True)
        result = subprocess.run(['make'], capture_output=True, text=True, timeout=60)
        if result.returncode != 0:
            print(f"Build failed: {result.stderr}")
            sys.exit(1)
    except subprocess.TimeoutExpired:
        print("Build timed out")
        sys.exit(1)
    except Exception as e:
        print(f"Build error: {e}")
        sys.exit(1)
    
    # Run tests
    tester = XV6Tester(timeout=args.timeout, quiet=args.quiet)
    success = tester.run_specific_test(args.testrex)
    
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()