import re


def test_broker_url_format(broker):
    assert re.fullmatch(r"ws://127\.0\.0\.1:\d+", broker), broker
