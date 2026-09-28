import time
import luos_engine as luos


def test_init_then_start_stop_cleanly():
    luos.init()
    svc = luos.create_service(type=4, alias="noop")
    luos.start()
    time.sleep(0.1)
    luos.stop()
    # After stop, starting again should work.
    luos.start()
    time.sleep(0.05)
    luos.stop()


def test_handler_not_required():
    svc = luos.create_service(type=4, alias="silent")
    luos.start()
    time.sleep(0.05)
    luos.stop()
