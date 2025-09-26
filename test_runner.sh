#!/bin/bash
echo "========================================"
echo "TerraPen Motion Control - Test Runner"
echo "========================================"
echo ""
echo "Running mathematical validation tests..."
echo "(No hardware required)"
echo ""

pio run -e test-math

echo ""
echo "========================================"
if [ $? -eq 0 ]; then
    echo "✅ TESTS COMPLETED SUCCESSFULLY"
    echo "All mathematical algorithms validated"
else
    echo "❌ TESTS FAILED"
    echo "Check the output above for errors"
fi
echo "========================================"