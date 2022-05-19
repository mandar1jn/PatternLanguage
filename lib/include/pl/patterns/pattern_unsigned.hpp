#pragma once

#include <pl/patterns/pattern.hpp>

namespace pl {

    class PatternUnsigned : public Pattern {
    public:
        PatternUnsigned(Evaluator *evaluator, u64 offset, size_t size, u32 color = 0)
            : Pattern(evaluator, offset, size, color) { }

        [[nodiscard]] std::unique_ptr<Pattern> clone() const override {
            return std::unique_ptr<Pattern>(new PatternUnsigned(*this));
        }

        u128 getValue() {
            u128 data = 0;
            this->getEvaluator()->readData(this->getOffset(), &data, this->getSize());
            return pl::changeEndianess(data, this->getSize(), this->getEndian());
        }

        [[nodiscard]] std::string getFormattedName() const override {
            switch (this->getSize()) {
                case 1:
                    return "u8";
                case 2:
                    return "u16";
                case 4:
                    return "u32";
                case 8:
                    return "u64";
                case 16:
                    return "u128";
                default:
                    return "Unsigned data";
            }
        }

        [[nodiscard]] bool operator==(const Pattern &other) const override { return areCommonPropertiesEqual<decltype(*this)>(other); }

        void accept(PatternVisitor &v) override {
            v.visit(*this);
        }

        std::string getFormattedValue() override {
            auto data = this->getValue();
            return this->formatDisplayValue(fmt::format("{:d} (0x{:0{}X})", data, data, 1 * 2), data);
        }
    };

}