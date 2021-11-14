Import("env")

# Python callback
def on_upload(source, target, env):
    print(source, target)
    firmware_path = str(source[0]).replace('\\', '/')
    # do something
    env.Execute("\"C:\\Program Files\\Git\\bin\\bash.exe\" -c \"/c/Users/G.TERRANOVA/Projects/esp-link/esp-link-v3.0.14-g963ffbb/avrflash 192.168.1.66 "+firmware_path+"\"")

env.Replace(UPLOADCMD=on_upload)