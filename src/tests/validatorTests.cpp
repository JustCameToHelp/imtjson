/*
 * validatorTests.cpp
 *
 *  Created on: 19. 12. 2016
 *      Author: ondra
 */

#include <iostream>
#include <fstream>
#include "testClass.h"
#include "../immujson/validator.h"
#include "../immujson/object.h"
#include "../immujson/string.h"

using namespace json;


void ok(std::ostream &out, bool res) {
	if (res) out <<"ok"; else out << "fail";
}


void vtest(std::ostream &out, Value def, Value test) {
	Validator vd(def);
	if (vd.validate(test)) {
		out << "ok";
	} else {
		out << vd.getRejections().toString();
	}

}

void runValidatorTests(TestSimple &tst) {

	tst.test("Validator.string","ok") >> [](std::ostream &out) {
		vtest(out,Object("","string"),"aaa");
	};
	tst.test("Validator.not_string","[[[],\"string\"]]") >> [](std::ostream &out) {
		vtest(out,Object("","string"),12.5);
	};
	tst.test("Validator.number","ok") >> [](std::ostream &out) {
		vtest(out,Object("","number"),12.5);
	};
	tst.test("Validator.not_number","[[[],\"number\"]]") >> [](std::ostream &out) {
		vtest(out,Object("","number"),"12.5");
	};
	tst.test("Validator.boolean","ok") >> [](std::ostream &out) {
		vtest(out,Object("","boolean"),true);
	};
	tst.test("Validator.not_boolean","[[[],\"boolean\"]]") >> [](std::ostream &out) {
		vtest(out,Object("","boolean"),"w2133");
	};
	tst.test("Validator.array","ok") >> [](std::ostream &out) {
		vtest(out,Object("",{array,"number"}),{12,32,87,21});
	};
	tst.test("Validator.arrayMixed","ok") >> [](std::ostream &out) {
		vtest(out,Object("",{array,"number","boolean"}),{12,32,87,true,12});
	};
	tst.test("Validator.array.fail","[[[1],\"number\"]]") >> [](std::ostream &out) {
		vtest(out,Object("",{array,"number"}),{12,"pp",32,87,21});
	};
	tst.test("Validator.arrayMixed-fail","[[[2],\"number\"],[[2],\"boolean\"]]") >> [](std::ostream &out) {
		vtest(out,Object("",{array,"number","boolean"}),{12,32,"aa",87,true,12});
	};
	tst.test("Validator.array.limit","ok") >> [](std::ostream &out) {
		vtest(out,Object("",{{array,"number"},{"maxsize",3}}),{10,20,30});
	};
	tst.test("Validator.array.limit-fail","[[[],[\"maxsize\",3]]]") >> [](std::ostream &out) {
		vtest(out,Object("",{"all",{array,"number"},{"maxsize",3}}),{10,20,30,40});
	};
	tst.test("Validator.tuple","ok") >> [](std::ostream &out) {
		vtest(out, Object("", { {3},"number","string","string" }), { 12,"abc","cdf" });
	};
	tst.test("Validator.tuple-fail1","[[[3],[[3],\"number\",\"string\",\"string\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { {3},"number","string","string" }), { 12,"abc","cdf",232 });
	};
	tst.test("Validator.tuple-fail2","[[[2],\"string\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", { {3},"number","string","string" }), { 12,"abc" });
	};
	tst.test("Validator.tuple-optional","ok") >> [](std::ostream &out) {
		vtest(out, Object("", { {3},"number","string",{"string","optional"} }), { 12,"abc" });
	};
	tst.test("Validator.tuple-fail3","[[[1],\"string\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", { {3},"number","string","string" }), { 12,21,"abc" });
	};
	tst.test("Validator.tuple+","ok") >> [](std::ostream &out) {
		vtest(out, Object("", { {2},"number","string","number" }), { 12,"abc",11,12,13,14 });
	};
	tst.test("Validator.object", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", Object("aaa","number")("bbb","string")("ccc","boolean")), Object("aaa",12)("bbb","xyz")("ccc",true));
	};
	tst.test("Validator.object.failExtra", "[[[],{\"aaa\":\"number\",\"bbb\":\"string\",\"ccc\":\"boolean\"}]]") >> [](std::ostream &out) {
		vtest(out, Object("", Object("aaa", "number")("bbb", "string")("ccc", "boolean")), Object("aaa", 12)("bbb", "xyz")("ccc", true)("ddd",12));
	};
	tst.test("Validator.object.failMissing", "[[[\"bbb\"],\"string\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", Object("aaa", "number")("bbb", "string")("ccc", "boolean")), Object("aaa", 12)("ccc", true));
	};
	tst.test("Validator.object.okExtra", "ok") >> [](std::ostream &out) {
		vtest(out, Object("",  Object("aaa", "number")("bbb", "string")("ccc", "boolean")("%","number")), Object("aaa", 12)("bbb", "xyz")("ccc", true)("ddd", 12));
	};
	tst.test("Validator.object.extraFailType", "[[[\"ddd\"],\"number\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", Object("aaa", "number")("bbb", "string")("ccc", "boolean")("%", "number")), Object("aaa", 12)("bbb", "xyz")("ccc", true)("ddd", "3232"));
	};
	tst.test("Validator.object.okMissing", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", Object("aaa", "number")("bbb", {"string","optional"})("ccc", "boolean")("%","number")), Object("aaa", 12)("ccc", true));
	};
	tst.test("Validator.userClass", "ok") >> [](std::ostream &out) {
		vtest(out, Object("test", { "string","number" })("", Object("aaa", "test")("bbb","test")), Object("aaa", 12)("bbb","ahoj"));
	};
	tst.test("Validator.recursive", "ok") >> [](std::ostream &out) {
		vtest(out, Object("test", { "string",{array,"test"} })("", { array, "number","test" }), { 10,{{"aaa"}} });
	};

	tst.test("Validator.recursive.fail", "[[[],\"number\"],[[],\"string\"],[[0,1,0],[[],\"test\"]],[[0,1,0],\"test\"],[[0,1],\"test\"],[[0],\"test\"],[[],\"test\"]]") >> [](std::ostream &out) {
		vtest(out, Object("test", { "string",{ array,"test" } })("", { "number","test" }), { { "ahoj",{ 12 } } });
	};
	tst.test("Validator.range", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, {">",0,"<",10} ,"string" } ), { "ahoj",4 });
	};
	tst.test("Validator.range.fail", "[[[1],\"string\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", { array, {">",0,"<",10},"string" }), { "ahoj",15 });
	};
	tst.test("Validator.range.multiple", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, {">",0,"<",10},{">=","A","<=","Z" } }), { "A",5 });
	};
	tst.test("Validator.range.multiple.fail", "[[[0],[\">\",0,\"<\",10]],[[0],[\">=\",\"A\",\"<=\",\"Z\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { array, {">",0,"<",10},{">=","A","<=","Z" } }), { "a",5 });
	};
	tst.test("Validator.fnAll", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "all","hex",">=","0","<=","9", "#comment" } ), "01255");
	};
	tst.test("Validator.fnAll.fail", "[[[],[\"all\",\"hex\",\">=\",\"0\",\"<=\",\"9\",\"#comment\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "all","hex", ">=","0","<=","9", "#comment"  }), "A589");
	};
	tst.test("Validator.fnAll.fail2", "[[[],\"hex\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "all","hex",{ ">=","0","<=","9" } }), "lkoo");
	};
	tst.test("Validator.prefix.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "prefix","abc." }), "abc.123");
	};
	tst.test("Validator.prefix.fail", "[[[],[\"prefix\",\"abc.\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "prefix","abc." }), "abd.123");
	};
	tst.test("Validator.suffix.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "suffix",".abc" }), "123.abc");
	};
	tst.test("Validator.suffix.fail", "[[[],[\"suffix\",\".abc\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "suffix",".abc" }), "123.abd");
	};
	tst.test("Validator.prefix.tonumber.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "prefix","abc.", {"tonumber",{">",100}} }), "abc.123");
	};
	tst.test("Validator.suffix.tonumber.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "suffix",".abc",{ "tonumber",{ ">",100 } } }), "123.abc");
	};
	tst.test("Validator.comment.any", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "#comment","any" }), 123);
	};
	tst.test("Validator.comment.any2", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "#comment","any" }), "3232");
	};
	tst.test("Validator.comment.any.fail", "[[[],\"#comment\"],[[],\"any\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "#comment","any" }), undefined);
	};
	tst.test("Validator.enum1", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", {"'aaa","'bbb","'ccc" }), "aaa");
	};
	tst.test("Validator.enum2", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "'aaa","'bbb","'ccc" }), "bbb");
	};
	tst.test("Validator.enum3", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "'aaa","'bbb","'ccc" }), "ccc");
	};
	tst.test("Validator.enum4", "[[[],\"'aaa\"],[[],\"'bbb\"],[[],\"'ccc\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "'aaa","'bbb","'ccc" }), "ddd");
	};
	tst.test("Validator.enum5", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { 111,222,333 }), 111);
	};
	tst.test("Validator.enum6", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { 111,222,333 }),222);
	};
	tst.test("Validator.enum7", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { 111,222,333 }), 333);
	};
	tst.test("Validator.enum8", "[[[],[111,222,333]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { 111,222,333 }), "111");
	};
	tst.test("Validator.set1", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, 1,2,3 }), { 1,2 });
	};
	tst.test("Validator.set2", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, 1,2,3 }), { 1,3 });
	};
	tst.test("Validator.set3", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, 1,2,3 }), array);
	};
	tst.test("Validator.set4", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, 1,2,3 }), { 2,3 });
	};
	tst.test("Validator.set5", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, 1,2,3 }), { 2, 3,3 });
	};
	tst.test("Validator.set6", "[[[1],[[],1,2,3]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { array, 1,2,3 }), { 2,4});
	};
	tst.test("Validator.prefix-array.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "prefix",{"a","b"} }), { "a","b","c","d" });
	};
	tst.test("Validator.prefix-array.fail", "[[[],[\"prefix\",[\"a\",\"b\"]]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "prefix",{ "a","b" } }), { "a","c","b","d" });
	};
	tst.test("Validator.suffix-arrau.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { "suffix",{ "c","d" } }), { "a","b","c","d" });
	};
	tst.test("Validator.suffix-array.fail", "[[[],[\"suffix\",[\"c\",\"d\"]]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { "suffix",{ "c","d" } }), { "a","c","b","d" });
	};
	tst.test("Validator.key.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", Object("%", { "all",{ "key",{"all","lowercase",{"maxsize",2} } },"number" })), Object("ab", 123)("cx", 456));
	};
	tst.test("Validator.key.fail", "[[[\"abc\"],[\"maxsize\",2]]]") >> [](std::ostream &out) {
		vtest(out, Object("", Object("%", { "all",{ "key",{ "all","lowercase",{ "maxsize",2 } } },"number" })), Object("abc", 123)("cx", 456));
	};
	tst.test("Validator.base64.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", "base64"), "flZhTGlEYVRvUn5+flRlc1R+fg==");
	};
	tst.test("Validator.base64.fail1", "[[[],\"base64\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "base64"), "flZhTGl@YVRvUn5+flRlc1R+fg==");
	};
	tst.test("Validator.base64.fail2", "[[[],\"base64\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "base64"), "flZhTGlEYVRvUn5+flRlc1R+f===");
	};
	tst.test("Validator.base64.fail3", "[[[],\"base64\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "base64"), "flZhTGlEYVRvUn5+flRlc1R+f==");
	};
	tst.test("Validator.base64url.ok", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", "base64url"), "flZhTGlEYVRvUn5_flRlc1R_fg==");
	};
	tst.test("Validator.base64url.fail1", "[[[],\"base64url\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "base64url"), "flZhTGlEYVRvUn5+flRlc1R+fg==");
	};
	tst.test("Validator.not1", "ok") >> [](std::ostream &out) {
		vtest(out, Object("", { array, {"not",1,2,3} }), { 10,20 });
	};
	tst.test("Validator.not2", "[[[0],[\"not\",1,2,3]]]") >> [](std::ostream &out) {
		vtest(out, Object("", { array,{ "not",1,2,3 } }), { 1,3 });
	};
	tst.test("Validator.datetime","ok") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2016-12-20T12:38:00Z");
	};
	tst.test("Validator.datetime.fail1","[[[],\"datetime\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2016-13-20T12:38:00Z");
	};
	tst.test("Validator.datetime.fail2","[[[],\"datetime\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2016-00-20T12:38:00Z");
	};
	tst.test("Validator.datetime.leap.fail","[[[],\"datetime\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2015-02-29T12:38:00Z");
	};
	tst.test("Validator.datetime.leap.ok","ok") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2016-02-29T12:38:00Z");
	};
	tst.test("Validator.datetime.fail3","[[[],\"datetime\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2016-01-29T25:38:00Z");
	};
	tst.test("Validator.datetime.fail4","[[[],\"datetime\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2016-12-20T12:70:00Z");
	};
	tst.test("Validator.datetime.fail5","[[[],\"datetime\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "2016-12-20T12:38:72Z");
	};
	tst.test("Validator.datetime.fail6","[[[],\"datetime\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "datetime"), "K<oe23HBY932pPLO(*JN");
	};
	tst.test("Validator.date.ok","ok") >> [](std::ostream &out) {
		vtest(out, Object("", "date"), "1985-10-17");
	};
	tst.test("Validator.date.fail","[[[],\"date\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "date"), "1985-10-17 EOX");
	};
	tst.test("Validator.time.ok","ok") >> [](std::ostream &out) {
		vtest(out, Object("", "timez"), "12:50:34Z");
	};
	tst.test("Validator.time.fail","[[[],\"timez\"]]") >> [](std::ostream &out) {
		vtest(out, Object("", "timez"), "A2xE0:34Z");
	};
	tst.test("Validator.customtime.ok","ok") >> [](std::ostream &out) {
		vtest(out, Object("", {"datetime","DD.MM.YYYY hh:mm:ss"} ), "02.04.2004 12:32:46");
	};
	tst.test("Validator.customtime.fail","[[[],[\"datetime\",\"DD.MM.YYYY hh:mm:ss\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("", {"datetime","DD.MM.YYYY hh:mm:ss"} ), "2016-12-20T12:38:72Z");
	};
	tst.test("Validator.entime.ok1","ok") >> [](std::ostream &out) {
		vtest(out, Object("tm",{ {"datetime","M:mm"},{"datetime","MM:mm"} })("",{{"suffix","am","tm"},{"suffix","pm"}}), "1:23am");
	};
	tst.test("Validator.entime.ok2","ok") >> [](std::ostream &out) {
		vtest(out, Object("tm",{ {"datetime","M:mm"},{"datetime","MM:mm"} })("",{{"suffix","am","tm"},{"suffix","pm"}}), "11:45pm");
	};
	tst.test("Validator.entime.fail1","[[[],[\"datetime\",\"M:mm\"]],[[],[\"datetime\",\"MM:mm\"]],[[],\"tm\"],[[],[\"suffix\",\"pm\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("tm",{ {"datetime","M:mm"},{"datetime","MM:mm"} })("",{{"suffix","am","tm"},{"suffix","pm"}}), "13:23am");
	};
	tst.test("Validator.entime.fail2","[[[],[\"suffix\",\"am\",\"tm\"]],[[],[\"suffix\",\"pm\"]]]") >> [](std::ostream &out) {
		vtest(out, Object("tm",{ {"datetime","M:mm"},{"datetime","MM:mm"} })("",{{"suffix","am","tm"},{"suffix","pm"}}), "2:45xa");
	};
	tst.test("Validator.selfValidate", "ok") >> [](std::ostream &out) {
		std::ifstream fstr("src/immujson/validator.json", std::ios::binary);
		Value def = Value::fromStream(fstr);
		Validator vd(def);
		if (vd.validate(def)) {
			out << "ok";
		}
		else {
			out << vd.getRejections().toString();
		}
	};
}


