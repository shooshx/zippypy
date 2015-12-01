// Copyright 2015 by Intigua, Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "myTest.h"

#include "../src/VarArray.h"
#include <vector>
#include <memory>

using namespace std;


TEST(VarArray, realloc_movedCorrectly)
{
    VarArray<string, 3> v;
    v.push_back("aa");
    v.push_back("bb");
    ASSERT_EQ(2, v.size());
    ASSERT_EQ("aa", v[0]);
    ASSERT_EQ("bb", v[1]);

    v.push_back("cc");
    v.push_back("dd");
    ASSERT_EQ(4, v.size());

    ASSERT_EQ("aa", v[0]);
    ASSERT_EQ("bb", v[1]);
    ASSERT_EQ("cc", v[2]);
    ASSERT_EQ("dd", v[3]);

    v.clear();
    ASSERT_EQ(0, v.size());

    v.push_back("ee");
    ASSERT_EQ(1, v.size());
    ASSERT_EQ("ee", v[0]);
}

template<typename T, typename FT>
vector<T> toVector(const FT& from) {
    vector<T> r;
    from.foreach([&](const T& v) { r.push_back(v); });
    return r;
}
template<typename FT>
void addValues(FT& to, int count) {
    for (int i = 0; i < count; ++i)
        to.push_back(std::to_string(i));
}

TEST(VarArray, popBackRealloc_movedCorrectly)
{
    VarArray<string, 3> v;
    addValues(v, 5);
    v.pop_back();
    v.pop_back();
    v.pop_back();
    ASSERT_EQ(2, v.size());
    ASSERT_THAT(toVector<string>(v), ElementsAre("0", "1"));
    v.pop_back();
    v.pop_back();
    ASSERT_EQ(0, v.size());

    addValues(v, 10);
    v.pop_back();
    v.pop_back();
    v.pop_back();
    v.pop_back();
    v.pop_back();
    ASSERT_THAT(toVector<string>(v), ElementsAre("0", "1", "2", "3", "4"));
}

TEST(VarArray, insert_movedCorrectly)
{
    VarArray<string, 3> v;
    v.push_back("1");
    v.push_back("2");
    v.insert(v.begin(), "000");
    ASSERT_EQ(3, v.size());
    ASSERT_THAT(toVector<string>(v), ElementsAre("000", "1", "2"));

    v.insert(v.begin() + 2, "333");
    v.insert(v.end(), "444");
    ASSERT_EQ(5, v.size());
    ASSERT_THAT(toVector<string>(v), ElementsAre("000", "1", "333", "2", "444"));
}

TEST(VarArray, iterators)
{
    VarArray<string, 3> v;
    v.push_back("1");
    v.push_back("2");
    v.insert(v.begin(), "000");
    ASSERT_THAT(v, ElementsAre("000", "1", "2"));

    vector<string> tmp;
    for (auto it = v.begin(); it != v.end(); ++it) {
        tmp.push_back(*it);
    }
    ASSERT_THAT(tmp, ElementsAre("000", "1", "2"));
    tmp.clear();

    // const iterators
    const VarArray<string, 3>& constV = v;
    v.push_back("4");
    ASSERT_THAT(v, ElementsAre("000", "1", "2", "4"));
    for (auto it = constV.begin(); it != constV.end(); ++it) {
        tmp.push_back(*it);
    }
    ASSERT_THAT(tmp, ElementsAre("000", "1", "2", "4"));

    // algorithms
    std::reverse(v.begin(), v.end());
    ASSERT_THAT(v, ElementsAre("4", "2", "1", "000"));

    // copy via c'tor
    vector<string> vCopy(v.begin(), v.end());
    ASSERT_THAT(vCopy, ElementsAre("4", "2", "1", "000"));
}



class TestObj {
public:
    TestObj() :m_x(0) {
        ++s_ctorEmpty;
    }
    explicit TestObj(int x) :m_x(x){
        ++s_ctorInit;
    }
    TestObj(const TestObj& t) :m_x(t.m_x) {
        ++s_ctorCopy;
    }
    ~TestObj() {
        ++s_dtors;
    }
    int m_x;

    static void zeroCounts() {
        s_ctorEmpty = 0; s_ctorCopy = 0; s_dtors = 0, s_ctorInit = 0;
    }
    static int aliveObjects() {
        return s_ctorEmpty + s_ctorCopy + s_ctorInit - s_dtors;
    }
    static int s_ctorEmpty, s_ctorCopy, s_dtors, s_ctorInit;
};
int TestObj::s_ctorEmpty = 0, TestObj::s_ctorCopy = 0, TestObj::s_dtors = 0, TestObj::s_ctorInit = 0;


TEST(VarArray, realloc_allDestroyed)
{
    TestObj::zeroCounts();
    VarArray<TestObj, 3> v;
    v.push_back(TestObj(11));
    v.push_back(TestObj(22));
    v.push_back(TestObj(33));
    ASSERT_EQ(3, TestObj::aliveObjects());
    v.push_back(TestObj(44));
    ASSERT_EQ(4, TestObj::aliveObjects());
}

TEST(VarArray, popBackRealloc_allDestroyed)
{
    TestObj::zeroCounts();
    VarArray<TestObj, 3> v;
    v.push_back(TestObj(11));
    v.push_back(TestObj(22));
    v.push_back(TestObj(33));
    ASSERT_EQ(3, TestObj::aliveObjects());
    v.pop_back();
    ASSERT_EQ(2, TestObj::aliveObjects());

    v.push_back(TestObj(333));
    v.push_back(TestObj(44));
    ASSERT_EQ(4, TestObj::aliveObjects());

    v.push_back(TestObj(55));
    v.push_back(TestObj(66));
    v.push_back(TestObj(77));
    ASSERT_EQ(7, TestObj::aliveObjects());

    v.pop_back();
    v.pop_back();
    ASSERT_EQ(5, TestObj::aliveObjects());

    v.pop_back();
    v.pop_back();
    v.pop_back();
    v.pop_back();
    v.pop_back();
    ASSERT_EQ(0, TestObj::aliveObjects());

}


TEST(VarArray, resize_objectsDestroyed)
{
    TestObj::zeroCounts();
    VarArray<TestObj, 3> v;
    v.resize(2);
    ASSERT_EQ(2, TestObj::aliveObjects());
    ASSERT_EQ(2, v.size());
    v.resize(4);
    ASSERT_EQ(4, TestObj::aliveObjects());
    ASSERT_EQ(4, v.size());

    v.push_back(TestObj(11));
    v.push_back(TestObj(22));
    v.push_back(TestObj(33));
    v.push_back(TestObj(44));
    v.push_back(TestObj(55));

    ASSERT_EQ(9, v.size());
    ASSERT_EQ(9, TestObj::aliveObjects());

    v.resize(4);
    ASSERT_EQ(4, v.size());
    ASSERT_EQ(4, TestObj::aliveObjects());

}


typedef std::shared_ptr<string> str_ptr;

TEST(VarArray, insert_allDestroyed)
{
    {
        TestObj::zeroCounts();
        VarArray<TestObj, 3> v;
        v.push_back(TestObj(11));
        v.push_back(TestObj(22));
        v.insert(v.begin() + 1, TestObj(000));
        ASSERT_EQ(3, TestObj::aliveObjects());
        v.insert(v.begin() + 1, TestObj(333));
        ASSERT_EQ(4, TestObj::aliveObjects());
    }

    VarArray<str_ptr, 3> v;
    v.push_back(str_ptr(new string("11")));
    v.push_back(str_ptr(new string("22")));
    v.insert(v.begin() + 1, str_ptr(new string("000")));
    v.foreach([](const str_ptr& p) {
        ASSERT_EQ(p.use_count(), 1);
    });
    v.insert(v.begin() + 1, str_ptr(new string("333")));
    v.insert(v.end(), str_ptr(new string("444")));
    v.foreach([](const str_ptr& p) {
        ASSERT_EQ(p.use_count(), 1);
    });

    v.insert(v.end() - 1, str_ptr(new string("555")));
    v.insert(v.end() - 1, str_ptr(new string("666")));
    v.insert(v.end() - 1, str_ptr(new string("777")));

    v.foreach([](const str_ptr& p) {
        ASSERT_EQ(p.use_count(), 1);
    });

    v.pop_back();
    v.pop_back();
    v.pop_back();

    v.foreach([](const str_ptr& p) {
        ASSERT_EQ(p.use_count(), 1);
    });
}


