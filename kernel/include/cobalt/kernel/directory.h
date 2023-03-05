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

#ifndef KERNEL_DIRECTORY_H
#define KERNEL_DIRECTORY_H

#include <cobalt/kernel/vnode.h>

class DirectoryVnode : public Vnode {
public:
    DirectoryVnode(const Reference<DirectoryVnode>& parent, mode_t mode,
            dev_t dev);
    ~DirectoryVnode();
    Reference<Vnode> getChildNode(const char* name) override;
    Reference<Vnode> getChildNode(const char* path, size_t length) override;
    size_t getDirectoryEntries(void** buffer, int flags) override;
    int link(const char* name, const Reference<Vnode>& vnode) override;
    off_t lseek(off_t offset, int whence) override;
    int mkdir(const char* name, mode_t mode) override;
    int mount(FileSystem* filesystem) override;
    bool onUnlink(bool force) override;
    Reference<Vnode> open(const char* name, int flags, mode_t mode) override;
    int rename(const Reference<Vnode>& oldDirectory, const char* oldName,
            const char* newName) override;
    Reference<Vnode> resolve() override;
    int symlink(const char* linkTarget, const char* name) override;
    int unlink(const char* path, int flags) override;
    int unmount() override;
private:
    Reference<Vnode> getChildNodeUnlocked(const char* name, size_t length);
    bool isAncestor(const Reference<Vnode>& vnode);
    int linkUnlocked(const char* name, size_t length,
            const Reference<Vnode>& vnode);
    int unlinkUnlocked(const char* path, int flags);
public:
    size_t childCount;
private:
    Reference<Vnode>* childNodes;
    char** fileNames;
    FileSystem* mounted;
protected:
    Reference<DirectoryVnode> parent;
};

#endif
