#include "VitalSigns.hpp"

namespace NeuraDoc {

bool VitalSigns::isValid() const {
    return heartRate > 0 && heartRate < 300 &&
           systolicBP > 0 && systolicBP < 300 &&
           diastolicBP > 0 && diastolicBP < 200 &&
           oxygenSaturation >= 0 && oxygenSaturation <= 100 &&
           temperature > 30 && temperature < 45 &&
           respiratoryRate > 0 && respiratoryRate < 60;
}

bool VitalSigns::isCritical() const {
    return heartRate < 40 || heartRate > 140 ||
           systolicBP > 180 || diastolicBP > 120 ||
           oxygenSaturation < 90 ||
           temperature < 35 || temperature > 40 ||
           respiratoryRate < 8 || respiratoryRate > 30;
}

} // namespace NeuraDoc
