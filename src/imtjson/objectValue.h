#pragma once

#include <vector>
#include "basicValues.h"

namespace json {

	class Object;

	class ObjectValue : public AbstractObjectValue {
	public:

		ObjectValue(const std::vector<PValue> &value);
		ObjectValue(std::vector<PValue> &&value);

		virtual std::size_t size() const override;
		virtual const IValue *itemAtIndex(std::size_t index) const override;
		virtual bool enumItems(const IEnumFn &) const override;
		virtual const IValue *member(const StringView<char> &name) const override;

		StringView<PValue> getItems() const { return v; }
		virtual bool getBool() const override {return true;}

	protected:
		std::vector<PValue> v;



	};


}
