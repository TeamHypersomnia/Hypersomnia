from wetransfer import TransferApi
import sys

x = TransferApi(sys.argv[2])

print( x.upload_file(sys.argv[1], "test upload") )
