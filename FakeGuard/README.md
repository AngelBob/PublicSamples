# FakeGuard

FakeGuard is an RSIGuard exectuable replacement. RSIGuard is a good thing, please use it whenever possible.  RSIGuard is a PITA on machines that are being reached via remote desktop when RSIGuard is in use on the accessing machine. Uninstalling RSIGuard my work, but then again, it may not. This executable can be copied over the top of the executable in the RSIGuard installation folder (C:\Program Files (x86)\RSIGuard).

Once copied, the auto-start for RSIGuard will launch this app instead of the original. This app will start, sit in a blocking wait for five minutes, then exit. This appears to be enough to appease the RSIGuard Gods, and no more annoying RSI break popups on the remote machine.

**NOTE:** it may be necessary to re-copy this executable periodically as RSIGuard gets updated/reinstalled.