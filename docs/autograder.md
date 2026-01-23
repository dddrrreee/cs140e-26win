# Student set-up for autograding

Basically, we need you to do two things:
 1. Send us an SSH public key
 2. Add another remote to your clone of the cs140e-26win repo

When you push your local cs140e-26win clone to our remote, autograding will happen automatically, though you _will_ need to specify which lab you want graded, like in the following example:
```shell
git push --upstream gradecope -o 1-trusting-trust
```

## Quick guide for SSH key generation

First, you'll generate a public-private key pair using `ssh-keygen`. Just run the following command and fill in the information.
```shell
ssh-keygen -t ed25519
```
By default, this will create two files:
 1. `~/.ssh/id_ed25519`
 2. `~/.ssh/id_ed25519.pub` <- this is your public key; SEND THIS ONE TO US: [https://forms.gle/Fqu8SDLL8DwRbYKf7](https://forms.gle/Fqu8SDLL8DwRbYKf7)
Note that we don't actually care what your SSH key is named; `id_ed25519` is the default for `ssh-keygen -t ed25519` and we are using it solely for demonstration purposes.

> [!WARNING]
> PLEASE PLEASE PLEASE DO NOT SEND US YOUR PRIVATE KEY!!!! WE DO NOT WANT IT!!!!

## Adding the remote to your repo

SSH keygen only generates a keypair; it doesn't actually let your system know about the new key, so you'll need to inform it yourself. This step is necessary for `git`
```shell
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519
```

Once we've received your public key, we'll generate a remote repository for you on our server. You will be able to add this remote to your repository like so:
```shell
git remote add gradecope ssh://<YOUR SUNET>@cs140e.gradecope.eeschal.net/~/gradecope-repo/.git
```

## Checking if the autograder recognizes you

Run the following
```shell
ssh <sunet>@cs140e.gradecope.eeschal.net gradecope-ctl hi
```
You should see
```shell
Hello, <sunet>!
```

If you see that, you're ready to go. If not, we may have not imported you yet (it's a manual process).

## Making submissions

```shell
git push --upstream gradecope -o 5-threads
```

## Using the autograder ctl

In general, invoke the autograder ctl with

```shell
ssh <sunet>@cs140e.gradecope.eeschal.net gradecope-ctl <command args>
```

### Viewing history

To see the most recent run for each lab

```
gradecope-ctl history
```

and for a specific lab

```
gradecope-ctl history <lab>
```

### Checking work

To see the detailed status plus a little log

```
gradecope-ctl status <lab> <id>
```

and to see all the logs

```
gradecope-ctl log <lab> <id>
```


