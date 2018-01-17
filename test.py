
DATABASE = 'test.db'

fail_template = """
Test name: {}
Expected:
{}END

Result:
{}END
"""

class Test:
    def __init__(self):
        self.tests = 0
        self.failures = []
        self.testname = None
        self.result = None
        self._skip = False
    
    def start(self):
        from time import time
        self.begin_time = time()
    
    def stop(self):
        import sys
        self.finish()
        sys.exit(0)
    
    def __call__(self, name):
        self.testname = name
    
    def skip(self):
        self._skip = True
    
    def eq(self, expected):
        if self._skip:
            self._skip = False
            return
        fail = (self.testname, expected, self.result)
        self.tests += 1
        self.testname = None
        if self.result == expected: return
        self.failures.append(fail)
    
    def finish(self):
        from time import time
        sec = time() - self.begin_time
        print 'Finished in {:.2} seconds'.format(sec)
        
        fails = len(self.failures)
        stat = (self.tests, fails)
        print '{} examples, {} failures'.format(*stat)
        
        if not fails: return
        print '\nFailures:'
        for data in self.failures:
            print fail_template.format(*data)


test = Test()

def expect(result):
    test.result = result
    return test

def run_script(commands, persistent = False):
    from subprocess import Popen, PIPE, STDOUT
    x = ['./d', DATABASE]
    p = Popen(x, stdin=PIPE, stdout=PIPE, stderr=STDOUT)
    stdout, stderr = p.communicate(commands)
    if not persistent: clean()
    return stdout

def run_persistent(commands):
    return run_script(commands, True)

def clean():
    import os
    if os.access(DATABASE, os.W_OK):
        os.unlink(DATABASE)


test.start()
clean()

test('inserts and retrieves a row')
result = run_script("""\
insert 1 user person@google.com
select
.exit
""")
expect(result).eq("""\
db > Executed.
db > (1, user, person@google.com)
Executed.
db > """)


test('prints error message when table is full')
data = []
query = 'insert {} user person@google.com'
for i in range(1401):
    data.append(query.format(i))
data.append('.exit\n')
query = '\n'.join(data)
result = run_persistent(query).split('\n')[-2]
expect(result).eq('db > Need to implement splitting a leaf node.')
clean()


test('allow inserting maximum length strings')
long_username = 'a' * 32
email = 'osiyuk@google.com'
long_email = 'b' * (255 - len(email)) + email
result = run_script("""\
insert 1 {} {}
select
.exit
""".format(long_username, long_email))
expect(result).eq("""\
db > Executed.
db > (1, {}, {})
Executed.
db > """.format(long_username, long_email))


test('prints error message if strings are too long')
long_username = 'a' * 32
email = 'osiyuk@google.com'
long_email = 'b' * (256 - len(email)) + email
result = run_script("""\
insert 1 {} {}
.exit
""".format(long_username, long_email))
expect(result).eq("""\
db > Error: string is too long.
db > """
.format(long_username, long_email))


test('prints error message if id is negative')
result = run_script("""\
insert -1 stack foo@google.com
.exit
""")
expect(result).eq("""\
db > Error: id must be positive.
db > """)


test('persistent data after process exit')
clean()
run_persistent("""\
insert 1 user osiyuk@google.com
.exit
""")
result = run_persistent("""\
select
.exit
""")
expect(result).eq("""\
db > (1, user, osiyuk@google.com)
Executed.
db > """)
clean()


test('prints error message if id is duplicate')
result = run_script("""\
insert 1 user person@example.com
insert 1 user person@example.com
select
.exit
""")
expect(result).eq("""\
db > Executed.
db > Error: duplicate key.
db > (1, user, person@example.com)
Executed.
db > """)

test.finish()

