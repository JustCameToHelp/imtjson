#pragma once

#include <vector>
#include "value.h"

namespace json {

	extern uintptr_t maxPrecisionDigits;


	template<typename Fn>
	class Serializer {
	public:



		Serializer(const Fn &target) :target(target) {}

		void serialize(const Value &obj);
		void serialize(const IValue *ptr);
		void serialize2(const IValue *ptr);

		void serializeObject(const IValue *ptr);
		void serializeArray(const IValue *ptr);
		void serializeNumber(const IValue *ptr);
		void serializeString(const IValue *ptr);
		void serializeBoolean(const IValue *ptr);
		void serializeNull();

		void serializeKeyValue(const IValue *ptr);

	protected:
		Fn target;

		void write(const StringRef<char> &text);
		void writeUnsigned(std::uintptr_t value);
		void writeUnsignedRec(std::uintptr_t value);
		void writeSigned(std::intptr_t value);
		void writeDouble(double value);
		void writeUnicode(unsigned int uchar);
		void writeString(const StringRef<char> &text);

		std::vector<const IValue *> linkRegister;

		bool findLink(const IValue *link) const;
	};

	class SerializerError :public std::runtime_error {
	public:
		SerializerError(std::string msg) :std::runtime_error(msg) {}
	};



	template<typename Fn>
	inline void Value::serialize(const Fn & target) const
	{
		Serializer<Fn> serializer(target);
		return serializer.serialize(*this);
	}

	template<typename Fn>
	inline void Serializer<Fn>::serialize(const Value & obj)
	{
		serialize((const IValue *)(obj.getHandle()));
		linkRegister.clear();
	}

	template<typename Fn>
	inline void Serializer<Fn>::serialize(const IValue * ptr)
	{
		if (ptr->flags() & mutableLink) {
			if (findLink(ptr)) {
				serialize2(NullValue::getNull());
			} else {
				linkRegister.push_back(ptr);
				serialize2(ptr->unproxy());
				linkRegister.pop_back();
			}
		} else {
			serialize2(ptr);
		}
	}
	template<typename Fn>
	inline void Serializer<Fn>::serialize2(const IValue * ptr)
	{
		switch (ptr->type()) {
		case object: serializeObject(ptr); break;
		case array: serializeArray(ptr); break;
		case number: serializeNumber(ptr); break;
		case string: serializeString(ptr); break;
		case boolean: serializeBoolean(ptr); break;
		case null: serializeNull(); break;
		}


	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeKeyValue(const IValue * ptr) {
		StringRef<char> name = ptr->getMemberName();
		writeString(name);
		target(':');
		serialize(ptr);

	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeObject(const IValue * ptr)
	{
		target('{');
		bool comma = false;
		auto fn = [&](const IValue *v) {
			if (comma) target(','); else comma = true;
			serializeKeyValue(v);
			return true;
		};
		ptr->enumItems(EnumFn<decltype(fn)>(fn));
		target('}');
	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeArray(const IValue * ptr)
	{
		target('[');
		bool comma = false;
		auto fn = [&](const IValue *v) {
			if (comma) target(','); else comma = true;
			serialize(v);
			return true;
		};
		ptr->enumItems(EnumFn<decltype(fn)>(fn));
		target(']');
	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeNumber(const IValue * ptr)
	{
		ValueTypeFlags f = ptr->flags();
		if (f & numberUnsignedInteger) writeUnsigned(ptr->getUInt());
		else if (f & numberInteger) writeSigned(ptr->getInt());
		else writeDouble(ptr->getNumber());			
	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeString(const IValue * ptr)
	{
		StringRef<char> str = ptr->getString();
		writeString(str);
	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeBoolean(const IValue * ptr)
	{
		if (ptr->getBool() == false) write("false"); else write("true");
	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeNull()
	{
		write("null");
	}

	template<typename Fn>
	inline void Serializer<Fn>::write(const StringRef<char>& text)
	{
		for (auto &&x : text) target(x);
	}

	template<typename Fn>
	inline void Serializer<Fn>::writeUnsignedRec(std::uintptr_t value) {
		if (value) {
			writeUnsignedRec(value / 10);
			target('0' + (value % 10));
		}
	}

	template<typename Fn>
	inline void Serializer<Fn>::writeUnsigned(std::uintptr_t value)
	{
		if (value) writeUnsignedRec(value);
		else target('0');
	}

	template<typename Fn>
	inline void Serializer<Fn>::writeSigned(std::intptr_t value)
	{
		if (value < 0) {
			target('-');
			writeUnsigned(-value);
		}
		else {
			writeUnsigned(value);
		}
	}


	template<typename Fn>
	inline void Serializer<Fn>::writeDouble(double value)
	{
		//calculate exponent of value
		//123897 -> 5 (1.23897e5)
		//0.001248 -> 3 (1.248e-3)
		double fexp = floor(log10(fabs(value)));
		//convert it to integer
		std::intptr_t iexp = (std::intptr_t)fexp;
		//if exponent is in some reasonable range, set iexp to 0
		if (iexp > -3 && iexp < 8) {
			iexp = 0;
		}
		else {
			//otherwise normalize number to be between 1 and 10
			value = value * pow(0.1, iexp);
		}		
		double fint;
		//separate number to integer and fraction
		double frac = abs(modf(value, &fint));
		//integer will fit to intptr, so convert it directly
		writeSigned(std::intptr_t(fint));
		//if frac is not zero (exactly)
		if (frac != 0.0) {
			//put dot
			target('.');

			double fractMultiply = pow(10, maxPrecisionDigits);
			//multiply fraction by maximum fit to integer
			std::uintptr_t m = (std::uintptr_t)floor(frac * fractMultiply +0.5);
			//remove any rightmost zeroes
			while (m && (m % 10) == 0) m = m / 10;
			//write final number
			writeUnsigned(m);
		}
		//if exponent is set
		if (iexp) {
			//put E
			target('E');
			//write signed exponent
			writeSigned(iexp);
		}
		//all done
	}

	static inline void notValidUTF8(const std::string &text) {
		throw SerializerError(std::string("String is not valid UTF-8: ") + text);
	}

	template<typename Fn>
	inline void Serializer<Fn>::writeUnicode(unsigned int uchar) {		
		target('\\');
		target('u');
		const char hex[] = "0123456789ABCDEF";
		for (unsigned int i = 0; i < 4; i++) {
			unsigned int b = (uchar >> ((3 - i) * 4)) & 0xF;
			target(hex[b]);
		}
	}


	template<typename Fn>
	inline void Serializer<Fn>::writeString(const StringRef<char>& text)
	{
		target('"');
		unsigned int uchar = 0;
		unsigned int extra = 0;
		for (auto&& c : text) {
			if ((c & 0x80) == 0x80) {
				if ((c & 0xC0) == 0x80) {
					if (extra) {
						uchar = (uchar << 6) | (c & 0x3F);
						extra--;
						if (extra == 0) writeUnicode(uchar);
					}
					else {
						notValidUTF8(text);
					}
				}
				else {
					if (extra) {
						notValidUTF8(text);
					}
					if ((c & 0xe0) == 0xc0) {
						extra = 1;
						uchar = c & 0x1F;
					}
					else if ((c & 0xf0) == 0xe0) {
						extra = 2;
						uchar = c & 0x0F;
					}
					else if ((c & 0xf8) == 0xf0) {
						extra = 3;
						uchar = c & 0x07;
					}
					else {
						notValidUTF8(text);
					}
				}
			}
			else {
				switch (c) {
				case '\\':
				case '/':
				case '"':target('\\'); target(c); break;
				case '\f':target('\\'); target('f'); break;
				case '\b':target('\\'); target('b'); break;
				case '\r':target('\\'); target('r'); break;
				case '\n':target('\\'); target('n'); break;
				case '\t':target('\\'); target('t'); break;
				default:if (c < 32) writeUnicode(c);else target(c); break;
				}
			}
		}
		if (extra) {
			notValidUTF8(text);
		}
		target('"');
	}

	template<typename Fn>
	inline bool Serializer<Fn>::findLink(const IValue* link) const {
		for (auto &&x: linkRegister)
			if (x == link) return true;
		return false;
	}

}

