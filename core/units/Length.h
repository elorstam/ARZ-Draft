#pragma once

namespace arz::core {

class Length final {
public:
    static Length fromMillimeters(double value);
    static Length fromCentimeters(double value);
    static Length fromMeters(double value);

    [[nodiscard]] double millimeters() const;
    [[nodiscard]] double centimeters() const;
    [[nodiscard]] double meters() const;

private:
    explicit Length(double millimeters);

    double millimeters_{0.0};
};

}
