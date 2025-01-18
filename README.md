# Dot Dot OS

This will be a OS based on per-app security, instead of the usual per-user system most OSes use.

The OS will enforce strict W^X (you cannot execute what you can write to). 

The OS will be based on the hardened linux kernel.

The OS will enforce a permission system to limit any potential malware.

The OS should not allow any application, and especially not a system one, to become comprised by a malicious user or application. It can do this by enforcing stict MAC on all functionality, even default ones like Wifi. (To prevent a system app from being RCEd / exploited into sending a reverse shell).

The OS should heavily restrict cross-app communication, and require both apps to explicitly allow each other to communicate.



# Init:

Init is the only process allowed to have UID and GID 0, and is the most privileged process on the system.

First, it checks /secure/ota/update.tar for a update file. If it finds it, it will check with the GPG public key at /system/update.key

If the update is signed with the distro's key, then remount all folders rw, and apply the update, delete the update file, and reboot.

If there is no update, It spawns the Zygote process first, sets it's UID and GID to 1.

Then it loads all Kernel Modules.

It will then create a FIFO file at /secure/init to listen for commands like shutdown and reboot. These commands must not take any user input, and must be as simple as possible to prevent code exec as init (worst case scenario).



# Permission system:

Permissions will be given by supplemental GIDs. Therefore, SGID will be blocked via a kernel module. Non-system apps cannot achieve any GID under 1500 to create a seperation between installable and system permissions.

There are 3 types of permissions:

System - These are the most restricted permissions, and can only be accessed by system apps.

Requested - These permissions require the user to manually grant them (via a popup)

Declarative - These permissions just need to be declared, and are generally extremely low-risk permissions that you don't need to be picky about.



# Groups:

- System (GID 0) -- This is a shared user and group. We should minimize usage of this as much as possible. This will be responsible for running essential services, like init.

- Zygote (GID 1) -- Zygote is the most privileged interactable process. It starts the settings manager, app manager, desktop, etc.

- SettingsManager (GID 2) -- Manages settings in /secure/settings.db, and non-ro props.

- OTA (GID 3) -- Downloads OTAs and reboots.

- AppManager (GID 5) -- Manages everything to do with applications, including installing, uninstalling, and changing permissions. Obviously, it cannot modify system applications, nor remount /system/ as RW.

- Graphics (GID 25) -- Has access to the framebuffer, and is responsible for rendering the GUIs of this OS.

- Wifi (GID 26) -- Responsible for wifi stuff like securely storing wifi passwords and other wifi stuff.

- Mount (GID 1005) -- Used by system apps to allow them to mount, unmount, etc. to /mnt/.

- Everybody (GID 1500) -- Granted to every user. Doesn't grant any permissions, and is only used in DAC to allow all apps to access stuff, as files all have a 0 in the others bit.

- Inet (GID 1501) -- Permission to access internet. This is a declarative permission.

- BindPort (GID 1502) -- Permission to be able to bind to and listen on non-privileged ports. This is a requested permission.

- HomeRW (GID 1750) -- Permission to be able to r/w to /home/*. This is a requested permission.



# Kernel modules:
Kernel modules will be used to 

- Globaly disable SGID

- Automatically set groups when SUID is used to switch apps

- Block anything from accessing /boot/, since it is a partition that by nessesity, is required to not have encryption.

- Block all mount requests outside /mnt/ (and only allow apps with the mount group to mount there).

- Do not allow files to set a "others" flag in chmod. All permissions must end with 0 in others (like 750). If you must give everyone access to a file, just set it's chown to (your user id):everybody and chmod to 750.



# FS:

/boot/ - Mounted RO, nosuid, noexec, nodev, etc.

/proc/, /dev/, /sys/ - all default linux, all mounted nosuid, noexec, etc.

/mnt/ - Mounted noexec, nosuid, nodev, etc. This is where USB drives and such are mounted.

/system/* - Mounted RO, contains system apps, /bin/, /lib[64]/*, build.prop, and the public key for checking OTA updates. This folder and all subfolders and files are only updated during OTAs, and shouldn't be edited locally.

/secure/* - Only writeable by system apps with the secure group. Contains settings.db, /ota/ (where update.tar goes) and /prop/* fs. Also contains /etc/

/secure/secbin/* - Secure Bin folder. Used to store essential files, such as init, zygote, etc.

/secure/ota/* - where update.tar goes. Only writable by system apps with the ota group

/secure/prop/* - automatically generated. This is where properties are stored. 

/secure/prop/ro/* - automatically gets populated from /system/build.prop, and is mounted RO. Cannot be modified at all. This is for properties you generally don't want to let the user modify, such as ro.secure (if set to 0, disables all security modules). These generally only get edited by OTAs.

/app/* - home folder of all non-system apps. Is the only folder outside of /system/* that allows executable files. Cannot be written to by anything except the app installer (system apps with installer group).

/data/* - where apps store data. Only place non-privileged apps can read write. Files in here cannot be executed to enforce W^X.

/home/* - contains download, documents, desktop, and other folders. Can only be read or written by apps with the group homerw.



# Properties:

ro/secure - defaults to 1, and if set to 0 completely disables all security kernel modules. This should never, ever, EVER be changed.

ro/debuggable - defaults to 0, but if set to 1 allows any system app to ptrace any non-system app. This should never be used outside local debugging.

graphics/headless - defaults to 0, but if set to 1 it will not start the graphics system.

# Apps:
Packages should include:
- packagename: a-z, 0-9, it can have _,-, and .'s, but not at the beginning or end.
- displayname: display name of the package, cannot have any control characters, newlines, carriage returns, or null characters. Optional.
- permissions: an array of permissions the package requests. Optional.
- gpgkey: public gpg key, used to validate updates to the app. If an app is updated, it must be signed by this gpg key.
- type: for now, the only types are 'app' and 'service'. Apps are the stuff you run and services can start on boot, but cannot have a GUI or user input, as they run in the background.
- interactable: array of packagenames that this package can interact with. Optional.
