#include "PrimitiveTypes.h"

namespace PrimitiveUtils {

bool IsValidPrimitiveType(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::TriangleStrip:
        case PrimitiveType::QuadStripInput:
        case PrimitiveType::QuadStrip:
        case PrimitiveType::TriangleList:
        case PrimitiveType::PointSprite:
        case PrimitiveType::LineStrip:
        case PrimitiveType::LineStripAlt:
        case PrimitiveType::ComplexPrimitive:
        case PrimitiveType::EndMarker:
        case PrimitiveType::Terminator:
            return true;
        default:
            return false;
    }
}

const char* GetTypeName(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::TriangleStrip:   return "TriangleStrip";
        case PrimitiveType::QuadStripInput:  return "QuadStripInput";
        case PrimitiveType::QuadStrip:       return "QuadStrip";
        case PrimitiveType::TriangleList:    return "TriangleList";
        case PrimitiveType::PointSprite:     return "PointSprite";
        case PrimitiveType::LineStrip:       return "LineStrip";
        case PrimitiveType::LineStripAlt:    return "LineStripAlt";
        case PrimitiveType::ComplexPrimitive: return "ComplexPrimitive";
        case PrimitiveType::EndMarker:       return "EndMarker";
        case PrimitiveType::Terminator:      return "Terminator";
        default:                             return "Unknown";
    }
}

bool IsControlConstant(PrimitiveType type) {
    return (type == PrimitiveType::EndMarker || 
            type == PrimitiveType::Terminator);
}

} // namespace PrimitiveUtils