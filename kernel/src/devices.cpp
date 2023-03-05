/***
 *                                                                                      
 *      ,ad8888ba,    ,ad8888ba,    88888888ba         db         88      888888888888  
 *     d8"'    `"8b  d8"'    `"8b   88      "8b       d88b        88           88       
 *    d8'           d8'        `8b  88      ,8P      d8'`8b       88           88       
 *    88            88          88  88aaaaaa8P'     d8'  `8b      88           88       
 *    88            88          88  88""""""8b,    d8YaaaaY8b     88           88       
 *    Y8,           Y8,        ,8P  88      `8b   d8""""""""8b    88           88       
 *     Y8a.    .a8P  Y8a.    .a8P   88      a8P  d8'        `8b   88           88       
 *      `"Y8888Y"'    `"Y8888Y"'    88888888P"  d8'          `8b  88888888888  88       
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Cobalt operating
 *  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <cobalt/poll.h>
#include <cobalt/kernel/console.h>
#include <cobalt/kernel/devices.h>
#include <cobalt/kernel/mouse.h>
#include <cobalt/kernel/panic.h>
#include <cobalt/kernel/process.h>
#include <cobalt/kernel/pseudoterminal.h>

class DevDir : public DirectoryVnode {
public:
    DevDir();
    int link(const char* name, const Reference<Vnode>& vnode) override;
    int mkdir(const char* name, mode_t mode) override;
    Reference<Vnode> open(const char* name, int flags, mode_t mode) override;
    int rename(const Reference<Vnode>& oldDirectory, const char* oldName,
            const char* newName) override;
    void setParent(const Reference<DirectoryVnode>& dir);
    int unlink(const char* path, int flags) override;
};

static DevDir _devDir;
static Reference<DevDir> devDir(&_devDir);
DevFS devFS;
const dev_t DevFS::dev = (dev_t) &_devDir;

class CharDevice : public Vnode {
public:
    CharDevice() : Vnode(S_IFCHR | 0666, DevFS::dev) {}

    short poll() override {
        return POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM;
    }

    ssize_t write(const void* /*buffer*/, size_t size, int /*flags*/) override {
        return size;
    }
};

class DevFull : public CharDevice {
public:
    ssize_t read(void* /*buffer*/, size_t /*size*/, int /*flags*/) override {
        return 0;
    }

    ssize_t write(const void* /*buffer*/, size_t size, int /*flags*/) override {
        if (size == 0) return 0;
        errno = ENOSPC;
        return -1;
    }
};

class DevNull : public CharDevice {
public:
    ssize_t read(void* /*buffer*/, size_t /*size*/, int /*flags*/) override {
        return 0;
    }
};

class DevZero : public CharDevice {
public:
    ssize_t read(void* buffer, size_t size, int /*flags*/) override {
        memset(buffer, 0, size);
        return size;
    }
};

class DevRandom : public CharDevice {
public:
    ssize_t read(void* buffer, size_t size, int /*flags*/) override {
        arc4random_buf(buffer, size);
        return size;
    }
};

class DevTty : public Vnode {
public:
    DevTty() : Vnode(S_IFCHR | 0666, DevFS::dev) {}

    Reference<Vnode> resolve() override {
        AutoLock lock(&Process::current()->jobControlMutex);
        return Process::current()->controllingTerminal;
    }
};

void DevFS::addDevice(const char* name, const Reference<Vnode>& vnode) {
    if (devDir->DirectoryVnode::link(name, vnode) < 0) {
        PANIC("Could not add device '/dev/%s'", name);
    }
}

Reference<Vnode> DevFS::getRootDir() {
    return devDir;
}

void DevFS::initialize(const Reference<DirectoryVnode>& rootDir) {
    devDir->setParent(rootDir);
    rootDir->mkdir("dev", 0755);
    Reference<Vnode> dir = rootDir->getChildNode("dev");
    if (!dir || dir->mount(this) < 0) {
        PANIC("Could not mount /dev filesystem.");
    }
    addDevice("console", console);
    addDevice("display", console->display);
    addDevice("full", xnew DevFull());
    mouseDevice = xnew MouseDevice();
    addDevice("mouse", mouseDevice);
    addDevice("null", xnew DevNull());
    addDevice("ptmx", xnew DevPtmx());
    addDevice("pts", xnew DevPts());
    Reference<Vnode> random = xnew DevRandom();
    addDevice("random", random);
    addDevice("tty", xnew DevTty());
    addDevice("urandom", random);
    addDevice("zero", xnew DevZero());

    // Update the /dev/display timestamp to avoid a 1970 timestamp.
    console->display->updateTimestampsLocked(true, true, true);
}

bool DevFS::onUnmount() {
    errno = EBUSY;
    return false;
}

DevDir::DevDir() : DirectoryVnode(nullptr, 0755, (uintptr_t) this) {

}

// Prevent the user from deleting devices or otherwise modifying /dev.

int DevDir::link(const char* /*name*/, const Reference<Vnode>& /*vnode*/) {
    errno = EROFS;
    return -1;
}

int DevDir::mkdir(const char* /*name*/, mode_t /*mode*/) {
    errno = EROFS;
    return -1;
}

Reference<Vnode> DevDir::open(const char* name, int flags, mode_t /*mode*/) {
    size_t length = strcspn(name, "/");
    Reference<Vnode> vnode = getChildNode(name, length);
    if (!vnode) {
        return nullptr;
    } else {
        if (flags & O_EXCL) {
            errno = EEXIST;
            return nullptr;
        } else if (flags & O_NOCLOBBER && S_ISREG(vnode->stats.st_mode)) {
            errno = EEXIST;
            return nullptr;
        }
    }

    return vnode;
}

int DevDir::rename(const Reference<Vnode>& /*oldDirectory*/,
        const char* /*oldName*/, const char* /*newName*/) {
    errno = EROFS;
    return -1;
}

void DevDir::setParent(const Reference<DirectoryVnode>& dir) {
    parent = dir;
}

int DevDir::unlink(const char* /*path*/, int /*flags*/) {
    errno = EROFS;
    return -1;
}
