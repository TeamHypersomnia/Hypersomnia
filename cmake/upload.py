from wetransfer import TransferApi
import sys

x = TransferApi(sys.argv[2])

print( x.upload_file("test upload", sys.argv[1]) )
