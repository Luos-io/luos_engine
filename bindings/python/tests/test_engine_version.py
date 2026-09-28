def test_engine_version_is_a_string():
    from luos_engine._version import ENGINE_VERSION
    assert isinstance(ENGINE_VERSION, str)
    assert ENGINE_VERSION  # non-empty
