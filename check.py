import os

def check(n, flush_type):
    count_fail = 0
    for _ in range(n):
        text = os.popen(f'./test {flush_type}').read()
        tmp  = text.split('\n')
        print(_, text)

        if tmp[1][-6:] != tmp[5][-6:] or tmp[2][-6:] != tmp[4][-6:]:
            count_fail += 1

    return count_fail

n   = 100
res = [0, 0, 0]

for _ in range(3):
    res[_] = check(n, _)

print(f'no flush,  {res[0]}/{n}')
print(f'sys_flush, {res[1]}/{n}')
print(f'sys_nop,   {res[2]}/{n}')

