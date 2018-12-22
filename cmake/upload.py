from py3wetransfer import Py3WeTransfer
import sys

x = Py3WeTransfer(sys.argv[2])

print( x.upload_file(sys.argv[1], "test upload") )
