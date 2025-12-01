import openstudio
import pytest

def openstudio_import():
    m = openstudio.model.Model()
    raise ValueError("Throwing on purpose")

def test_openstudio_import():
    openstudio_import()

if __name__ == "__main__":
    pytest.main([__file__, "--capture=no", "--verbose"])

