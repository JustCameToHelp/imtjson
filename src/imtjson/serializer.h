#pragma once

#include <cstdlib>
#include <vector>
#include "value.h"

namespace json {

	///Configures precision of floating numbers
	/** Global variable specifies count of decimal digits of float numbers. 
	The default value is 4 for 16bit platform, 8 for 32bit platform and 9 for
	64bit platform. It can be changed, however, maximum digits is limited
	to size of uintptr_t type. This limits count of digits max to 10digits on
	32bit platform and 21 digits on 64bit platform 
	*/
	extern uintptr_t maxPrecisionDigits;
	///Specifies default output format for unicode character during serialization
	/** If output format is not specified by serialization function. Default
	is emitEscaped*/
	extern UnicodeFormat defaultUnicodeFormat;


	template<typename Fn>
	class Serializer {
	public:



		Serializer(const Fn &target, bool utf8output) :target(target), utf8output(utf8output) {}

		void serialize(const Value &obj);
		virtual void serialize(const IValue *ptr);

		void serializeObject(const IValue *ptr);
		void serializeArray(const IValue *ptr);
		void serializeNumber(const IValue *ptr);
		void serializeString(const IValue *ptr);
		void serializeBoolean(const IValue *ptr);
		void serializeNull(const IValue *ptr);

		void serializeKeyValue(const IValue *ptr);

	protected:
		Fn target;
		bool utf8output;

		void write(const StringView<char> &text);
		void writeUnsigned(std::uintptr_t value);
		void writeUnsignedRec(std::uintptr_t value);
		void writeSigned(std::intptr_t value);
		void writeDouble(double value);
		void writeUnicode(unsigned int uchar);
		void writeString(const StringView<char> &text);

	};

	class SerializerError :public std::runtime_error {
	public:
		SerializerError(std::string msg) :std::runtime_error(msg) {}
	};



	template<typename Fn>
	inline void Value::serialize(const Fn & target) const
	{
		serialize(defaultUnicodeFormat, target);
	}

	template<typename Fn>
	inline void Value::serialize(UnicodeFormat format, const Fn & target) const
	{
		Serializer<Fn> serializer(target, format == emitUtf8);
		return serializer.serialize(*this);
	}

	template<typename Fn>
	inline void Serializer<Fn>::serialize(const Value & obj)
	{
		serialize((const IValue *)(obj.getHandle()));
	}

	template<typename Fn>
	inline void Serializer<Fn>::serialize(const IValue * ptr)
	{
		switch (ptr->type()) {
		case object: serializeObject(ptr); break;
		case array: serializeArray(ptr); break;
		case number: serializeNumber(ptr); break;
		case string: serializeString(ptr); break;
		case boolean: serializeBoolean(ptr); break;
		case null: serializeNull(ptr); break;
		case undefined: writeString("undefined"); break;
		}


	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeKeyValue(const IValue * ptr) {
		StringView<char> name = ptr->getMemberName();
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
		StringView<char> str = ptr->getString();
		writeString(str);
	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeBoolean(const IValue * ptr)
	{
		if (ptr->getBool() == false) write("false"); else write("true");
	}

	template<typename Fn>
	inline void Serializer<Fn>::serializeNull(const IValue *)
	{
		write("null");
	}

	template<typename Fn>
	inline void Serializer<Fn>::write(const StringView<char>& text)
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
		double frac = std::abs(modf(value, &fint));
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
			target('e');
			if (iexp > 0) target('+');
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
		//see: http://stackoverflow.com/questions/2965293/javascript-parse-error-on-u2028-unicode-character
		if (utf8output && uchar >=0x80 && uchar != 0x2028 && uchar != 0x2029) {
			if (uchar >= 0x80 && uchar <= 0x7FF) {
				target((char)(0xC0 | (uchar >> 6)));
				target((char)(0x80 | (uchar & 0x3F)));
			}
			else if (uchar >= 0x800 && uchar <= 0xFFFF) {
				target((char)(0xE0 | (uchar >> 12)));
				target((char)(0x80 | ((uchar >> 6) & 0x3F)));
				target((char)(0x80 | (uchar & 0x3F)));
			}
			else if (uchar >= 0x10000 && uchar <= 0x10FFFF) {
				target((char)(0xF0 | (uchar >> 18)));
				target((char)(0x80 | ((uchar >> 12) & 0x3F)));
				target((char)(0x80 | ((uchar >> 6) & 0x3F)));
				target((char)(0x80 | (uchar & 0x3F)));
			}
			else {
				notValidUTF8("");
			}
		}
		else {
			target('\\');
			target('u');
			const char hex[] = "0123456789ABCDEF";
			for (unsigned int i = 0; i < 4; i++) {
				unsigned int b = (uchar >> ((3 - i) * 4)) & 0xF;
				target(hex[b]);
			}
		}
	}


	template<typename Fn>
	inline void Serializer<Fn>::writeString(const StringView<char>& text)
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

}

