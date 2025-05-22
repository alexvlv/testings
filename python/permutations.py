#!/usr/bin/python3

from itertools import product, permutations

Print = False

def F1(x, y, z, w):
    r = (x <= y) == (w and (not z))
    if(Print): print("F1:",x, y, z, w,r)
    return r

def F2(x, y, z, w):
    r = (x <= y) and ((not w) == z)
    if(Print): print("F2:",x, y, z, w,r)
    return r

def Cond0(x, y, z, w):
    return not F2(x, y, z, w)

def Cond1(x, y, z, w):
    return not F1(x, y, z, w)

def Cond2(x, y, z, w):
    return not F1(x, y, z, w) and F2(x, y, z, w)


def Check(p, table):
    global Print
    #d0 = dict(zip(p, table[0]))
    #print (F2(**d0))
    #print(, F2(**dict(zip(p, table[0])))))
    Print = False
    c = Cond0(**dict(zip(p, table[0]))) and Cond1(**dict(zip(p, table[1]))) and Cond2(**dict(zip(p, table[2])))
    if c:
        Print = True
        Cond0(**dict(zip(p, table[0]))) and Cond1(**dict(zip(p, table[1]))) and Cond2(**dict(zip(p, table[2])))
        print (table)
    return c

def main():
    for p in permutations('xyzw'):
        #print(*p)
        for a, b, c in product([0, 1], repeat=3):
            #print(a,b,c)
            table = [[a, 1, 0, 1],
                     [b, 0, 0, 0],
                     [0, c, 0, 0]]
            if table[0] == table[1] or table[0] == table[2] or table[1] == table[2]:
                continue
            if Check(p, table):
                print(*p)
if __name__ == '__main__':
    main()


# https://alex-math.ru/gia/show/zadaniye-2-informatika-yege-polyakov-6616/
#if all(F1(**dict(zip(p, row))) == F2(**dict(zip(p, row))) for row in table):
#    print(*p)
# https://www.yaklass.ru/p/informatika/11-klass/informatcionnye-tekhnologii-7279409/bazy-dannykh-subd-6820711/re-418ca6bc-f78b-423a-9a14-ff4dcc06dbdc
