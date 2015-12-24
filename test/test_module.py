# -*- coding: utf-8 -*-

# Copyright 2015 by Intigua, Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import imped_module

#def testFormat():
#    print 'hello %d aaa %s bbbb' % (3, 'x');

#def testForBreak():
#    a = [2,4,'bb',8,12]
#    for x in a:
#        if x == 8:
#            break        
#        print x



def testMap():
    x = {"a": 42, 43:"b", 47:"c"}
    EQ(x.pop(47), "c")
    EQ(len(x), 2)
    x["a"] = 44
    x["b"] = 45
    print x["b"], x[43]
    y = {1: 42, 5:"b", 6:"asf", 47:"c"}

    EQ(x[u'b'], 45) # conversion in comparison


def testMapItter():
    map = {"a": 42, 43:"b"}
    counter = 0	
    for x in map:
        if counter == 0:
            EQ(x, "a")
            EQ(map[x], 42)
        if(counter == 1):
            EQ(x, 43)
            EQ(map[x], "b")
        counter += 1
    EQ(counter, 2)

    counter = 0	
    for k,v in map.iteritems():
        if counter == 0:
            EQ(k, "a")
            EQ(v, 42)
        if(counter == 1):
            EQ(k, 43)
            EQ(v, "b")
        counter += 1
    EQ(counter, 2)


def testStrMapItter(map):
    counter = 0;
    for x in map:
        if counter == 0:
            EQ(x , "a")
            EQ(map[x] ,42)
        if(counter == 1):
            EQ(x , "c")
            EQ(map[x] ,43)
        counter+=1
    EQ(counter,2)
    
            

#def lib(name):
#    def decor(cls):
#        setattr(cls, 'lib', name)
#        return cls
#    return decor
#@lib("bla")
#class TestClassDecor():
#    def tmethod(self):
#        print "Bla"



print 'toplvl'

def EQ(a, b):
    if a != b:
        raise str(a) + " != " + str(b)
def NEQ(a, b):
    if a == b:
        raise str(a) + " == " + str(b)
def TRUE(a):
    if not a:
        raise str(a) + " not True"
def FALSE(a):
    if a:
        raise str(a) + " not False"
def FAIL(msg):
    raise msg;

def testb(a, b):
    EQ(a, 3)
    EQ(b, 1)

def testf(a, b):
    EQ(a, 3)
    c = a + b
    EQ(c, 4)
    testb(a,b)

    EQ(ctest1(1, True, 'chello'), False)
    x = None
    y = 2222222222222222
    
def testEq():
    TRUE([] == [])
    TRUE([1,2] == [1,2])
    TRUE((1,2) == (1,2))
    FALSE([] == [1])
    TRUE(1 == 1)
    FALSE(1 == 2)
    FALSE(1 != 1)
    TRUE(1 != 2)
    TRUE('aa' == 'aa')
    FALSE('aa' == 'bb')
    TRUE('aa' != 'bb')
    FALSE('aa' != 'aa')
    
    class A:
        pass
    class B:
        pass
    FALSE(A == B)
    TRUE(A != B)

def testList():
    l = [1,2]
    EQ(len(l), 2)
    l[0] = 'one';
    EQ(l[0], 'one')
    EQ(l, ['one', 2])
    l.append(3)
    EQ(l, ['one', 2, 3])
    l.extend([4, '5'])
    EQ(l, ['one', 2, 3, 4, '5'])
    l.append(None)
    EQ(l[5], None)
    EQ(l, ['one', 2, 3, 4, '5', None])
    l.extend([None, 6])
    EQ(l, ['one', 2, 3, 4, '5', None, None, 6])
    
    EQ(l.pop(0), 'one')
    EQ(l, [2, 3, 4, '5', None, None, 6])    
    EQ(l.pop(2),4)
    EQ(l, [2, 3, '5', None, None, 6])
    
    
    l = [1,2,3,4]
    EQ(l[0], 1)
    EQ(l[3], 4)
    EQ(l[-1], 4)
    EQ(l[-4], 1)
    


def checkNumber(i, arg):
    if i == 1:
        return -2147483646  # HKEY_LOCAL_MACHINE = FFFFFFFF80000002
    if i == 2:
        return 4294967296   # 0x100000000
    if i == 3:
        return 0x7F00000000000000
    if i == 4:
        return 0xAF00000000000000
    if i == 11:
        TRUE(arg == -2147483646)
    if i == 12:
        TRUE(arg == 4294967296)   # 0x100000000
    if i == 13:
        TRUE(arg == 0x7F00000000000000)
    if i == 14:
        TRUE(arg == 0xAF00000000000000)
    
    
def testCircular():
    a1 = ['one', 'two']
    a2 = [a1, 'three']
    a1[0] = a2
    EQ(str(a1), "[[[...], 'three'], 'two']")
    # recursive comparison not implemented yet

def testFor():
    a = [2,4,'bb',8]
    s = ''
    for x in a:
        print x
        s += str(x)
    EQ(s, "24bb8")   
        
def testIntMathOps():
    a = 42
    EQ(a, 42)
    a += 1
    EQ(a, 43)
    a *= 2
    EQ(a, 86)
    a /= 3
    EQ(a, 28)
    a -= 19
    EQ(a, 9)
    b = a - 3
    EQ(b, 6)
    c = b * 8
    EQ(c, 48)
    b = c / 2
    EQ(b, 24)
    c = a + b
    EQ(c, 33)
    b = -c
    EQ(b, -33)
    a = +c
    EQ(a, 33)
    return (a,b,c)

def testStrMathOps():
    s = 'ab'
    EQ(s, 'ab')
    s *= 4
    EQ(s, 'abababab')
    s = s * 2
    EQ(s, 'abababababababab')
    s = 2 * s
    EQ(s, 'abababababababababababababababab')
    s = 'jk'
    EQ(s * -2, '')
    g = 'hg' + 'a'
    EQ(g, 'hga')
    g = g + 'b'
    EQ(g, 'hgab')
    return (s,g)

def testLogicOps():
    a = True
    b = False
    NEQ(a, b)
    c = a and b
    EQ(c, False)   
    b = a or c
    EQ(b, True)
    a = not b
    EQ(a, False)
    
    EQ(True and True, True)
    EQ(False and True, False)
    EQ(True and False, False)
    EQ(False and False, False)
    EQ(True or True, True)
    EQ(False or True, True)
    EQ(True or False, True)
    EQ(False or False, False)
    
    return (a,b,c)

def testFloatMath():
    a = 1.5
    a *= 2.0
    EQ(a, 3.0)
    #NEQ(a, 3) # can't compare, different from CPython!
    a += 1.1
    EQ(a, 4.1)
    a -= 0.2
    TRUE(a - 3.9 < 1e-6)
    a /= 0.6
    TRUE(a - 6.5 < 1e-6)
    a = -a
    TRUE(a + 6.5 < 1e-6)
    a = +a
    TRUE(a + 6.5 < 1e-6)
    return a        

def testIntFloatComprasions():
    TRUE(5 > 4.2)
    TRUE(5.6 > 4)
    TRUE(5==5.0)
    TRUE(5.0==5)
    FALSE(6==6.1)
    FALSE(6.1==6)
    TRUE(6.1>=5)
    FALSE(4.2>=5)
    
def testIntFloatBasicOperations():
    EQ(5.2+2, 7.2)
    EQ(2+5.2, 7.2)    
        
    EQ(6/1.5,4.0)
    EQ(0.5/2,0.25)
    
    EQ(3.2*1, 3.2)
    EQ(1*3.3, 3.3)
    
    EQ(15.5 - 5, 10.5)
    EQ(16-0.5,15.5)    
        
def testIs():
    a = None
    TRUE(a is None)
    FALSE(a is 1)
    TRUE(a is not 1)
    FALSE(a is 'a')
    TRUE(a is not 'a')
    b = 2
    FALSE(b is None)
    TRUE(b is not None)
    FALSE(b is 3-1) # different from CPython, for int < 256
    b += 1
    a = 'bla'
    FALSE(a is b)
    TRUE(a is not b)
    b = a
    TRUE(a is b)
    FALSE(a is not b)
        
        
def testBuiltInFuncs():
    EQ(len([1,2,3]), 3)
    EQ(len((True, 5)), 2)
    EQ(len('bla_bla'), 7)
    EQ(hash('aaabbb'), -1590267375)
    EQ(hash('bd'), 603887296)
    EQ(str(111), "111")
    EQ(hex(0x111), "0x111")
       
def testCfuncs():
    a = ctest1(1, True, "bla")
    EQ(a, False)
    b = ctest2(2, "bla2")
    EQ(b, None)
    c = ctest3(2, "bla3")
    EQ(c, "XX")
    z = ctest0()
    EQ(z, 1)
    m = ctest4(3.141, u"Walla")

    ctest5([1, 2, "abc"])
    ctest6([1, 2, 3])

    
def testCMethod(inst):
    inst.cmethod1('back-hello')
    x = inst.cmethod2()
    EQ(x, 2)
    x = inst.cmethod3(2, False, "BLAA")
    EQ(x, False)
    x = inst.cmethod4(3, True)
    EQ(x, 42)
    inst.cmethod5(3, "XXX", True)

def testSplit():
    EQ([1,2], [1,2])
    NEQ([1,2], [1,3])
    EQ([], [])
    NEQ([1], [1,2])
    
    EQ(" HelloxyzWorldxyzxyzBla".split("xyz"), [" Hello", "World", "", "Bla"])
    EQ(" HelloxyzWorldxyzxyzBlaxyz".split("xyz"), [" Hello", "World", '', "Bla", ""])
    EQ(" HelloxyzWorldxyzxyzBlaxyz".split("xyz", False), [" Hello", "World", "Bla"])

    EQ("".split("xyz"), [''])
    EQ("XXXabcXXX".split("XXX"), ["", "abc", ""])
    EQ("XXXabcXXX".split("XX"), ['', 'Xabc', 'X'])
    
    EQ(",,,,".split(","), ['', '', '', '', ''])
    EQ(",,,,".split(",", False), [])
    EQ(",a,".split(","), ['', "a", ''])
    EQ(",a,".split(",", True), ['', "a", ''])
    EQ(",a,".split(",", False), ["a"])
    EQ("b, c ,a, ".split(","), ["b", " c ", "a", " "])

    EQ("".split(), [])
    EQ(" hello  world \tbla  \n".split(), ["hello", "world", "bla"])
    EQ("\n a b c".split(), ["a", "b", "c"])
    
    print "OK"
    
def testUnicode():
    x = u'hello'
    TRUE(x.beginsWith(u'he'))
    TRUE(x == u'hello')
    print x
    EQ(x + u'bla', u'hellobla')
    EQ(x * 2, u'hellohello')
    EQ(len(u'bla'), 3)
    EQ(len(u''), 0)
    #print hash('a'), ' ', hash(u'a')
    EQ(hash('a'), hash(u'a'))
    
    TRUE(x.beginsWith('he'))
    a = u'he\u0583llo'
    print a
    TRUE(a.contains(u'e\u0583l'))
    FALSE(a.endswith(u'?llo'))
    TRUE(a.endswith('llo'))
    TRUE(a.iContains('LL'))
    FALSE(a.contains('Ll'))
    TRUE(u'xxx' == 'xxx')
    TRUE(u'a' < 'b')
    EQ('ababa'.split(u'a'), [u'', u'b', u'b', u''])
    EQ(u'ababa'.split('a'), [u'', u'b', u'b', u''])
    EQ('ababa'.split(u'a', False), [u'b', u'b'])
    
    a = 'aaa'
    EQ(a + 'bbb', 'aaabbb')
    EQ(a + u'bbb', u'aaabbb')
    a = u'aaa'
    EQ(a + 'bbb', u'aaabbb')
    EQ(a + u'bbb', u'aaabbb')    
    print "OK"
      
    
def testJoin():
    EQ(','.join(['a', 'b', 'c']), "a,b,c")
    EQ(','.join(['a']), 'a')
    a = 'd'
    TRUE(','.join([a]) is a)
    EQ('AA'.join(['1','2']), "1AA2")
    EQ('AA'.join([]), '')
    # unicode
    EQ(u','.join(['a', 'b', 'c']), u"a,b,c")
    EQ(u','.join([u'a', u'b', u'c']), u"a,b,c")
    EQ(u','.join(['a', u'b', 'c']), u"a,b,c")
    EQ(u','.join(['a', u'b', 'c']), u"a,b,c")
    
    EQ(u'AA'.join([]), u'')
    a = u'd'
    TRUE(','.join([a]) is a)
    print "OK join"

def r(x):
    return x
    
def testBitOp():
    EQ(r(3) | 4, 7)
    EQ(r(0x11) | 0x110, 0x111)
    EQ(r(0xFFFFFFFFF0000000) | 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF)

    EQ(r(3) & 4, 0)
    EQ(r(0x11) & 0x110, 0x10)
    EQ(r(0xFFFFFFFFF0000000) & 0xFFFFFFFF, 0xF0000000)
    
    EQ(r(3) ^ 4, 7)
    EQ(r(0x11) ^ 0x110, 0x101)
    EQ(r(0xFFFFFFFFF0000000) ^ 0xFFFFFFFF, 0xFFFFFFFF0FFFFFFF)

    EQ(r(1) << 63, 0x8000000000000000)
    EQ(r(3) << 4, 0x30)
    EQ(r(3) << 33, 0x600000000)
    
    EQ(r(0x8000000000000000) >> 63, 1)
    EQ(r(0x30) >> 4, 3)
    EQ(r(0x600000000) >> 33, 3)
    
    x = 3
    x |= 4
    EQ(x, 7)
    x &= 0xFE
    EQ(x, 6)
    x ^= 12
    EQ(x, 10)
    x <<= 4
    EQ(x, 0xa0)
    x >>= 4
    EQ(x, 10)
    
    x = 1
    EQ(~x, -2)
    x = 33
    EQ(~x, -34)
    x = -1
    EQ(~x, 0)
    x = 0
    EQ(~x, -1)
    x = 0xa1a1a1a1a1a1a1
    EQ(~x, -45495186823946658)
    print "OK"
    
    
def key_f(x, aa, bb, cc, a, b, c):
    EQ(x, 0)
    EQ(aa, a)
    EQ(bb, b)
    EQ(cc, c)
    
class KeyC():
    def __init__(self):
        self.bla = -1
        print "in init"
    def f(self, x, aa, bb, cc, a, b, c):
        EQ(self.bla, -1)
        EQ(x, 0)
        EQ(aa, a)
        EQ(bb, b)
        EQ(cc, c)


    
def testKeyWordArgs():
    key_f(0, 1, 2, 3, 1, 2, 3)
    print 1
    key_f(0, 1, 2, 3, c = 3, a = 1, b = 2)
    print 2
    key_f(0, 1, 2, 3, 1, b = 2, c = 3)
    print 3
    key_f(0, aa=1, bb=2, cc=3, a=1, b=2, c=3)
    a = 10
    b = 20
    key_f(0, a, b, 30, a=a, b=b, c=30)
    print 4
    
    i = KeyC()
    i.f(0, 1, 2, 3, 1, 2, 3)
    i.f(0, 1, 2, 3, c = 3, a = 1, b = 2)
    i.f(0, 1, 2, 3, 1, b = 2, c = 3)
    i.f(0, aa=1, bb=2, cc=3, a=1, b=2, c=3)
    i.f(0, a, b, 30, a=a, b=b, c=30)
    
    print "OK1"

    # test args that are given more than once
    #key_f(0, 1, 2, 3, c = 3, aa = 1, b = 2)
    
    # key that doesn't exist
    #key_f(0, 1, 2, 3, d = 3, a = 1, b = 2)
    # missing arg
    #key_f(0, 1, 2, 3, a = 1, b = 2)
    
def testCKeyWordArgs():
    cfunc_kwa(3, aa=1, bb='hello')
    

class InitTest():
    def __init__(self, a, b, c):
        print "initTest", a, b, c
        self.a = a
        self.b = b
        self.c = c
    
    
class StaticDataTest():
    STAT = 1
class StaticDataDerive(StaticDataTest):
    DDDSTAT = 2
    def __init__(self):
        self.bla = 3
    
    def method(self, a):
        EQ(self.bla, 3)
        EQ(a, 4)
    @staticmethod
    def smethod(a):
        EQ(a, 5)

class TestBase:
    def bpol(self):
        self.bla = 666
class TestDerived(TestBase):
    def pol(self):
        self.non = None
        print "hey", self.non

    
def testClass():
    t = InitTest(1,2,3);
    EQ(t.a, 1)
    EQ(t.b, 2)
    EQ(t.c, 3)

    EQ(StaticDataTest.STAT, 1)
    EQ(StaticDataDerive.STAT, 1)
    EQ(StaticDataDerive.DDDSTAT, 2)
    
    d = StaticDataDerive()
    EQ(d.bla, 3)
    EQ(d.STAT, 1)
    EQ(d.DDDSTAT, 2)
    d.method(4)
    d.smethod(5)
    StaticDataDerive.smethod(5)
    
    x = TestDerived()
    x.bpol()
    x.pol()
    print x.bla, x.non
    EQ(str(x.bla) + str(x.non), "666None")    

    # instance equality
    a = TestBase()
    b = TestBase()
    c = a
    TRUE(a == c)
    FALSE(a == b)
    FALSE(b == c)
    
externRef = None
def testMem(doLeakRef):
    # new object
    t = InitTest(1,2,3);
    x = TestDerived()

    if doLeakRef:
        global externRef
        externRef = "hello leak"
    
    # create cycle
    a = [0,0]
    b = [a,1]
    a[0] = b
    return a
    
def testImport():
    imped_module.hello()
    x = imped_module.HelloCls()
    x.helloo()
    
    
def testListCompr():
    l = [x*2 for x in [1,2,3]]
    EQ(l, [2,4,6])
    l = [x*2 if x < 5 else x*10 for x in [1,2,3,4,5,6,7,8,9]]
    EQ(l, [2, 4, 6, 8, 50, 60, 70, 80, 90])
    print "OK lstcmpr"
    

def testUnpack():
    x = [1,2]
    a,b = x
    EQ(a,1)
    EQ(b,2)
    a,b = [1,2]    
    EQ(a,1)
    EQ(b,2)

    x = [1,2,3]
    a,b,c = x
    EQ(a,1)
    EQ(b,2)
    EQ(c,3)
    a,b,c = [1,2,3]    
    EQ(a,1)
    EQ(b,2)
    EQ(c,3)

    a,b,c,d = [1,2,3,4]    
    EQ(a,1)
    EQ(b,2)
    EQ(c,3)
    EQ(d,4)

    x = [1,2,3,4]
    a,b,c,d = x
    EQ(a,1)
    EQ(b,2)
    EQ(c,3)
    EQ(d,4)
    print "OK unpack"
    

def strIter():
    r = []
    for x in 'abc':
        r.append(x)
    EQ(r, ['a', 'b', 'c'])
    r = []
    for x in u'abc':
        r.append(x)
    EQ(r, [u'a', u'b', u'c'])    
    print "OK strIter"
    
    s = "abcd"
    EQ(s[0], "a")
    EQ(s[3], "d")
    # throws s[4]
    EQ(s[-1], 'd')
    EQ(s[-4], 'a')
    
    s[0] = "E"  # different from CPython
    EQ(s, "Ebcd")
    s[3] = "R"
    EQ(s, "EbcR")
    # throws s[0] = "EX", s[0] = ''
    
    s = u"abcde"
    EQ(s[0], u'a')
    EQ(s[4], u'e')
    EQ(s[-1], u'e')
    
def testStrInOp():
    TRUE('aa' in 'baab')
    FALSE('aa' not in 'baab')
    TRUE('ac' not in 'baab')
    FALSE('ac' in 'baab')
    TRUE('' in 'x')
    FALSE('' not in 'x')
    FALSE('xxx' in 'a')
    
    TRUE(u'ab' in 'uabu')
    TRUE('ab' in u'uabu')
    TRUE(u'ab' in u'uabu')
    
def testTuple():
    x = (1,2)
    EQ(x, (1,2))
    EQ(x[0], 1)
    EQ(x[1], 2)
    
def testListInOp():
    TRUE(1 in [1,2,3])
    TRUE(1 in (1,2,3))
    TRUE(1 in [3,2,1])
    TRUE('aa' in [1,2,3,'aa'])
    TRUE([2] in [1,[2]])    
    FALSE(1 in [2,3])
    FALSE(1 in (2,3))
    FALSE(1 in [[1],2,3])
    FALSE([1] in [1,2,3])
    
    FALSE(1 not in [1,2,3])
    FALSE(1 not in (1,2,3))
    FALSE(1 not in [3,2,1])
    FALSE('aa' not in [1,2,3,'aa'])
    FALSE([2] not in [1,[2]])    
    TRUE(1 not in [2,3])
    TRUE(1 not in (2,3))
    TRUE(1 not in [[1],2,3])
    TRUE([1] not in [1,2,3])
    
    TRUE('b' in ['a', 'b', 'c'])
    TRUE(u'a' in ['a', 'b']);   
    TRUE('a' in [u'a', u'b']);   
    FALSE([] in [])
    TRUE([] in [[]])
    TRUE(1 not in [])
    FALSE(None in [])
    TRUE(None in [None])
    # None in None throws

    
def testStrDictInOp():
    ab = strdict(a=1, b=2)
    cb = strdict(c=1, b=2)
    TRUE('a' in ab)
    FALSE('a' in cb)
    FALSE('a' not in ab)
    TRUE('a' not in cb)
    
    FALSE(1 in ab)
    FALSE('a' in strdict())

def testStrDictSize():
    d = strdict()
    EQ(len(d), 0)
    for i in xrange(1,100):
        d[str(i)] = 1
        EQ(len(d), i)

    
def testStrDictSubScript():
    x = strdict()
    x['aaa'] = 2
    x['bbb'] = [1,2,3]
    EQ(x['aaa'], 2)
    EQ(x['bbb'], [1,2,3])
    x['c'] = None
    TRUE(x['c'] is None)
    
    y = strdict(x='hello', y='world')
    EQ(y['x'], 'hello')
    EQ(y['y'], 'world')
    y['x'] = 'bla'
    EQ(y['x'], 'bla')
    
    lk = y.keys()
    print lk
    TRUE('x' in lk)
    TRUE('y' in lk)
        
def testStrDictValuesFunc():
    x = strdict()
    x['a'] = 2
    x['b'] = [1,2,3]
    x['c'] = "3"
    values = x.values()
    TRUE(2 in values)
    TRUE([1,2,3] in values)
    TRUE("3" in values)
    
# ----------------------------------------------------------------    

def parseCharArray(envp):
    #envp = self.envp.get()
    rdbuf = AccessBuffer(envp)
    wrbuf = AccessBuffer(envp)
    savedRefs = [] # need to save refs to appended strings
    sp = rdbuf.readPtr()       
    while (sp != 0):
        sbuf = AccessBuffer(sp)
        s = sbuf.readCStr()
        
        #vs = vosTranslateFile(s)
        print s
        vs = s.lower()
        
        savedRefs.append(vs)
        wrbuf.writePtr(vs.c_ptr())
        sp = rdbuf.readPtr()
    charArrCall(envp)
    
 
def testAccessBuf(s): 
    wbuf = AccessBuffer(s.c_ptr())
    wbuf.writeInt(1, 100)
    #wbuf.writeInt(2, 200)
    wbuf.writeInt(4, 300)
    wbuf.writeInt(8, 400)
    
    rbuf = AccessBuffer(s.c_ptr())
    EQ(rbuf.readInt(1), 100)
    #EQ(rbuf.readInt(2), 200)
    EQ(rbuf.readInt(4), 300)
    EQ(rbuf.readInt(8), 400)
    print "testWriteBuf OK1"
    
    wbuf = AccessBuffer(s.c_ptr())
    wbuf.writeInt(1, 200)
    #wbuf.writeInt(2, 60000)
    wbuf.writeInt(4, 4000000000)
    
    rbuf = AccessBuffer(s.c_ptr())
    EQ(rbuf.readInt(1), -56)
    #EQ(rbuf.readInt(2), -5536)
    EQ(rbuf.readInt(4), -294967296)
    
    # seek
    xs = "abc12345\x006789defgABCD\x00";
    rbuf = AccessBuffer(xs.c_ptr())
    rbuf.seekBytes(AccessBuffer.ORIGIN_BEG, 2)
    a = rbuf.readCStr();
    EQ(a, "c12345")
    rbuf.seekBytes(AccessBuffer.ORIGIN_CUR, 1*AccessBuffer.SIZEOF_PTR)
    b = rbuf.readCStr();
    if AccessBuffer.SIZEOF_PTR == 4:
        EQ(b, "defgABCD")
    elif AccessBuffer.SIZEOF_PTR == 8:
        EQ(b, "ABCD")
    else:
        FAIL("unexpected SIZEOF_PTR")
        
    wxs = u"abcd\u0000efg\u0000"
    rbuf = AccessBuffer(wxs.c_ptr())
    a = rbuf.readWCStr();
    EQ(a, u"abcd")
    b = rbuf.readWCStr()
    EQ(b, u"efg")
    
    
def testBuildBuf(a, b, c):
    bbuf = BufferBuilder()
    bbuf.writePtr(a.c_ptr())
    bbuf.writePtr(b.c_ptr())
    bbuf.writePtr(c.c_ptr())
    bbuf.writeInt(8, 88888)
    bbuf.writeInt(4, 666)
    bbuf.writeInt(1, 111)
    # bbuf.writeInt(2, 777)

    return bbuf.str()


def testArgcArgv(argc, argv):
    rdbuf = AccessBuffer(argv)
    wrbuf = AccessBuffer(argv)
    count = 0;
    while (count < argc):
        sp = rdbuf.readPtr()
        sbuf = AccessBuffer(sp)
        s = sbuf.readCStr()
        us = s.lower()
        print s, "->", us
        wrbuf.writePtr(us.glob_ptr())
        count = count + 1
        
def testGetAttr():
    class Bla:
        hello = 1
        def mook():
            pass
        
    EQ(hasattr(Bla, "hello"), True)
    EQ(hasattr(Bla, "mook"), True)
    EQ(hasattr(Bla, "bla"), False)
    
    EQ(getattr(Bla, "hello"), 1)
    EQ(getattr(Bla, "hello", 2), 1)
    EQ(getattr(Bla, "bla", 2), 2)
    #getattr(Bla, "bla")  throws
    
    print "testGetAttr OK"
    
def testVarArgFunc(i):
    EQ(i.varArgFunc(), "0 ")
    EQ(i.varArgFunc(1, 2), "2 1 2 ")
    EQ(i.varArgFunc("bla", True, 1), "3 bla True 1 ")

    
def testException():
    raise "Blaa"


def gen():
    i = 1
    while i < 10:
        yield i
        i = i+1
    yield None
    yield "bla"
        
def testGen():
    x = []
    for i in gen():
        x.append(str(i))
    
    #x = list(gen());
    s = ' '.join(x)
    print s
    EQ(s, "1 2 3 4 5 6 7 8 9 None bla")
    
def instanceCObject():
    obj = CClass()
    
    return 1

def useCObject(obj):
    a = obj.getSomething()    
    return 1

def callWithInstance():
    obj = CClass3("Hello")
    id = gettingInstance(obj)
    EQ(id, "Hello")


class AttCls:
    clsData = 555

    def __init__(self):
        self.instanceData = 777
    def internalFunc(self):
        return 666;

    def __getattr__(self, name):
        if name == "data1":
            return "TheData"
        if name == "func1":
            return self.internalFunc
        else:
            raise AttributeError()

class NoAttCls:
    def func(self):
        return 333

def testGetAttr():
    a = AttCls()
    EQ(a.data1, "TheData")
    EQ(a.func1(), 666)

    TRUE(hasattr(a, "internalFunc"))
    TRUE(hasattr(a, "instanceData"))
    TRUE(hasattr(a, "clsData"))
    TRUE(hasattr(AttCls, "clsData"))
    TRUE(hasattr(AttCls, "internalFunc"))
    FALSE(hasattr(AttCls, "instanceData"))
    
    TRUE(hasattr(a, "data1")) # hasattr supports __getattr__
    TRUE(hasattr(a, "func1"))

    EQ(getattr(a, "instanceData"), 777)
    EQ(getattr(a, "clsData"), 555)
    EQ(getattr(a, "internalFunc")(), 666)

    EQ(getattr(a, "data1"), "TheData")
    EQ(getattr(a, "func1")(), 666)

    b = NoAttCls()
    TRUE(hasattr(b, "func"))
    FALSE(hasattr(b, "mook"))
    EQ(getattr(b, "func")(), 333)

    #with default from __getattr__
    EQ(getattr(a, "blabla", 901), 901)
    #default from normal attribute
    EQ(getattr(b, "blabla", 902), 902)

def testLogger():
	#simple validation, maybe add validation that logs are really written
    logging.debug('a', 5, 'b')
    logging.debug([5,6,7], "b")
    logging.error([5,6,7], {5:[1,2,3], 6:'a'})
    logging.error(AttCls(), 'a')
    logging.error('asdasdasd2354 1234123' + '123123asdf')
    

def testStringComparisons():
    TRUE('hello'.beginsWith('hell'))
    TRUE('hello'.startswith('hell'))
    FALSE('Hello'.beginsWith('hell'))
    TRUE('hello'.iBeginsWith('hell'))
    TRUE('HEllo'.iBeginsWith('hell'))
    TRUE('HEllo'.iBeginsWith('HELL'))        
    TRUE('hello'.contains('hell'))
    FALSE('hEllo'.contains('hell'))
    TRUE('hEllo'.iContains('heLL'))
    FALSE('hEllo'.contains('ello'))
    FALSE('hello'.contains('ellok'))
    FALSE('hell'.contains('hello'))
    TRUE('hEllo'.iContains('eLLO'))
    TRUE('blablablahello'.endswith('llo'))
    TRUE('blablablahello'.endsWith('llo'))
    FALSE('blablablahello'.endsWith('llO'))
    TRUE('blablablahellO'.endsWith('llO'))
    TRUE('blablablahello'.iEndsWith('llO'))
    FALSE('blablablahello'.iEndsWith('llOk'))
    TRUE('blabla'.equals('blabla'))
    FALSE('blablA'.equals('blabla'))
    FALSE('blabla'.equals('blablak'))
    FALSE('blablak'.equals('blabla'))
    TRUE('blablA'.iEquals('Blabla'))
    TRUE('blabla'.iEquals('blabla'))
    
    #windows only
#    TRUE('c:/bla/bla'.pathEquals(r'C:\bla\BLA'))
#    TRUE('c:/bla/bla'.pathEquals('c:/bla/bla'))
#    TRUE('c://bla/bla'.pathEquals('c:/bla//bla'))
#    FALSE('c:/blaa/bla'.pathEquals('c:/bla/bla'))
#    FALSE('c/bla/bla'.pathEquals('c:/bla/bla'))
#    TRUE(r'd:\bla/bla'.pathBeginsWith('d:/Bla'))
#    FALSE(r'd:\bla/bla'.pathBeginsWith('d:/Blab'))
#    TRUE(r'd:\blax/bla'.pathContains('/Blax'))
#    TRUE(r'd:\blax/BLa'.pathContains('x/bla'))
#    TRUE('c:/Bla/Bla'.pathEndsWith(r'a\bla'))
#    FALSE('c:/Bla/Bla'.pathEndsWith(r'a\bl'))

    TRUE(''.equals(''))
    TRUE(''.beginsWith(''))
    TRUE(''.endsWith(''))
    TRUE(''.contains(''))
    TRUE(''.iEquals(''))
    TRUE(''.iBeginsWith(''))
    TRUE(''.iEndsWith(''))
    TRUE(''.iContains(''))
#    TRUE(''.pathEquals(''))
#    TRUE(''.pathBeginsWith(''))
#    TRUE(''.pathEndsWith(''))
#    TRUE(''.pathContains(''))

def testRound():
    EQ(5, round(5.2))
    EQ(5, round(5))
    EQ(6, round(5.8))


def withVarPos1(a, b, *args):
    print "VarPos1 ", a, " ", b, " ", args
    EQ(a, 1)
    EQ(b, 2)
    EQ(args, ('Bla'))

def withVarPos2(*args):
    print "VarPos2 ", args
    EQ(args, (1, 2, 'Bla')) 

def withVarPos3(*args):
    print "VarPos3 ", args
    EQ(args, ())

class WithVarPos:
    def methWithVarPos(self, a, b, *args):
        print "methVarPos ", a, b, args
        EQ(a, 'a')
        EQ(b, 'b')
        EQ(args, ('c', 'd', 5))

def testVarPos():
    withVarPos1(1, 2, 'Bla')
    withVarPos2(1, 2, 'Bla')
    withVarPos3()
    wv = WithVarPos()
    wv.methWithVarPos('a', 'b', 'c', 'd', 5)


def loadRuntimeModule():
    #m = runtime_import("runtime_mod.py") # buildin function
    m = runtime_import("runtime_mod.json")
    print m
    return m

def testIntCast():
    EQ(int('3'), 3)
    EQ(int(u'42'), 42)
    EQ(int(True), 1)
    EQ(int(False), 0)
    EQ(int(3.1), 3)
    EQ(int(3.9), 3)

    EQ(bool(3), True)
    EQ(bool(0), False)
    EQ(bool(None), False)
    EQ(bool(''), False)
    EQ(bool('false'), False)
    EQ(bool('FalSe'), False)
    EQ(bool('True'), True)
    EQ(bool('xxx'), True)
    EQ(bool(u''), False)
    EQ(bool(u'false'), False)
    EQ(bool(u'FalSe'), False)
    EQ(bool(u'True'), True)
    EQ(bool(u'xxx'), True)
    EQ(bool(0.0), False)
    EQ(bool(0.1), True)
    
    EQ(bool([]), False)
    EQ(bool([1]), True)
    EQ(bool(()), False)
    EQ(bool((1,2)), True)
    EQ(bool({}), False)
    EQ(bool({1:2}), True)


mookInInit = []
mookInMeta = []

def myMeta(name, bases, methods): 
    global mookInMeta
    mookInMeta.append((name, bases, methods))
    return type(name, bases, methods) 

class Mook1:
    __metaclass__ = myMeta
    def __init__(self):
        global mookInInit
        mookInInit.append(1)
    def do1(self):
        return 1

class Mook2:
    __metaclass__ = myMeta
    def __init__(self):
        global mookInInit
        mookInInit.append(2)
    def do2(self):
        return 2        

class Mook3(Mook2):
    def __init__(self):
        global mookInInit
        mookInInit.append(3)
    def do3(self):
        return 3        


def testMetaClass():
    EQ(len(mookInMeta), 3)
    EQ(mookInMeta[0][0], "Mook1")
    EQ(mookInMeta[1][0], "Mook2")
    EQ(mookInMeta[2][0], "Mook3")
    
    m = Mook1()
    EQ(mookInInit, [1])
    EQ(m.do1(), 1)
    m = Mook2()
    EQ(mookInInit, [1, 2])
    EQ(m.do2(), 2)

    m = Mook3()
    EQ(mookInInit, [1, 2, 3])
    EQ(m.do2(), 2)
    EQ(m.do3(), 3)


class C:
    def __init__(self):
        self.a = 11
    def f(self, b, c):
        return (self.a, b, c)

class NotC:
    def __init__(self):
        self.a = 22

def testUnboundedMethod1():
    x = C()
    r = C.f(x, 2, 3)
    print r    
    EQ(r, (11, 2, 3))

def testUnboundedMethod2():
    x = NotC()
    r = C.f(x, 2, 3) # this does not work in CPython but here I allow it.
    print r
    EQ(r, (22, 2, 3))

def testClambda():
    ltest(1, "hello")


some_global = 1
class GlobalAccess:
    if some_global == 1:
        print "some_global", some_global
        def f1(self):
            return 1;

def testGlobalInClass():
    g = GlobalAccess()
    EQ(g.f1(), 1)



def testSubstr():
    s = "abcdefg"
    EQ(s[:], "abcdefg")

    EQ(s[0:2], "ab")
    EQ(s[0:6], "abcdef")
    EQ(s[0:7], "abcdefg")
    EQ(s[0:10], "abcdefg")
    EQ(s[0:0], "")
    EQ(s[1:2], "b")
    EQ(s[2:2], "")
    EQ(s[2:4], "cd")
    EQ(s[2:7], "cdefg")
    EQ(s[2:10], "cdefg")
    
    EQ(s[0:-1], "abcdef")
    EQ(s[2:-1], "cdef")
    EQ(s[2:-4], "c")
    EQ(s[2:-5], "")
    EQ(s[2:-6], "")
    EQ(s[-6:-1], "bcdef")
    EQ(s[-1:-20], "")
    EQ(s[-20:-1], "abcdef")
    EQ(s[-20:0], "")

    EQ(s[2:], "cdefg")
    EQ(s[-3:], "efg")
    EQ(s[:2], "ab")
    EQ(s[:-1], "abcdef")

    EQ(s[0:4:2], "ac") # start:end:step
    EQ(s[0:6:2], "ace")
    EQ(s[0:5:2], "ace")
    EQ(s[1:-2:2], "bd")
    EQ(s[1:-2:1], "bcde")
    EQ(s[-1:0:-1], "gfedcb")
    EQ(s[-1:0:-2], "gec")
    
    EQ(s[-20:20:2], "aceg")

    # everything frob above, just with an extra colon
    EQ(s[::], "abcdefg")

    EQ(s[0:2:], "ab")
    EQ(s[0:6:], "abcdef")
    EQ(s[0:7:], "abcdefg")
    EQ(s[0:10:], "abcdefg")
    EQ(s[0:0:], "")
    EQ(s[1:2:], "b")
    EQ(s[2:2:], "")
    EQ(s[2:4:], "cd")
    EQ(s[2:7:], "cdefg")
    EQ(s[2:10:], "cdefg")
    
    EQ(s[0:-1:], "abcdef")
    EQ(s[2:-1:], "cdef")
    EQ(s[2:-4:], "c")
    EQ(s[2:-5:], "")
    EQ(s[2:-6:], "")
    EQ(s[-6:-1:], "bcdef")
    EQ(s[-1:-20:], "")
    EQ(s[-20:-1:], "abcdef")
    EQ(s[-20:0:], "")

    EQ(s[2::], "cdefg")
    EQ(s[-3::], "efg")
    EQ(s[:2:], "ab")
    EQ(s[:-1:], "abcdef")    

    # TBD over,undershoot
    
def testStrip():
    EQ("  a ".strip(), "a")
    EQ("a ".strip(), "a")
    EQ("  a".strip(), "a")
    EQ("  a\t\n ".strip(), "a")
    EQ("  ".strip(), "")
    EQ("".strip(), "")

def testImportCallback():
    import imped_module2
    EQ(imped_module2.hello2(), 2)

def testDictCollision():
    d = {}
    d['a'] = 1
    h = hash('a')
    d[h] = 2
    EQ(d['a'], 1)
    EQ(d[h], 2)

def mkList(r):
    l = []
    for a in r:
        l.append(a)
    return l

def testXrange():
    EQ( mkList(xrange(0,10)), [0,1,2,3,4,5,6,7,8,9])
    EQ( mkList(xrange(0,0)), [])
    EQ( mkList(xrange(0,1)), [0])
    EQ( mkList(xrange(0,-2)), [])

    EQ( mkList(xrange(5)), [0,1,2,3,4])
    EQ( mkList(xrange(0)), [])
    EQ( mkList(xrange(-4)), [])

    EQ( mkList(xrange(0,4,2)), [0,2])
    EQ( mkList(xrange(0,5,2)), [0,2,4])
    EQ( mkList(xrange(1,5,2)), [1,3])
    EQ( mkList(xrange(1,6,2)), [1,3,5])
    EQ( mkList(xrange(1,6,10)), [1])
    EQ( mkList(xrange(1,12,10)), [1,11])
    EQ( mkList(xrange(1,12,-1)), [])
    EQ( mkList(xrange(1,-6,-1)), [1,0,-1,-2,-3,-4,-5])
    EQ( mkList(xrange(1,-6,-2)), [1,-1,-3,-5])