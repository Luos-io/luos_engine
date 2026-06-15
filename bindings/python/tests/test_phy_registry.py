from luos_engine import _registry


def test_phys_list_exists_and_starts_empty():
    assert _registry.PHYS == []


def test_clear_empties_phys():
    class _Fake:
        descriptor = None
        handle = None
        loop = lambda self=None: None
    _registry.PHYS.append(_Fake())
    assert len(_registry.PHYS) == 1
    _registry.clear()
    assert _registry.PHYS == []
