---
layout: default
title: Publish
parent: Release & Milestone Tasks
grand_parent: Contributors
nav_order: 90
---

<!--
© 2021 and later: Unicode, Inc. and others.
License & terms of use: http://www.unicode.org/copyright.html
-->

# Publish
{: .no_toc }

## Contents
{: .no_toc .text-delta }

1. TOC
{:toc}OnlineDemosHowToUpdate

---

## Create a release branch in GitHub

Once the branch is created, only changes necessary for the target release are
merged in from the trunk.

---

## Upgrade LocaleExplorer and other demos/samples

... to the ICU project site.

Build the icu-demos module following the README's. Update code and/or docs as
needed. "Reference" platforms for icu-demos are: RedHat Linux and win32. On Linux,
icu-demos is built against the "make install "'ed ICU. So, run ICU4C's configure
with --prefix=/some/where pointing to where ICU4C should be installed, and also
follow icu-demos's README.

Install the new locale explorer and other demos/samples onto the public demo
hosting site.

---

## ICU Collation Demo

Update the ICU collation demo's `index.html` with the new ICU version’s
available collators.

1.  Do a clean build (configure, make clean, make install, make check).
    1.  Otherwise, the data build may not pick up a new locale into the
        coll/res_index.txt file.
2.  Run [icu-demos >
    webdemo/collation/build.sh](https://github.com/unicode-org/icu-demos/blob/main/webdemo/collation/build.sh)
    (after modifying it for your system).
3.  Copy-paste the output `available-collators.txt` into `index.html`.
    1.  Or, easier: Use a GUI difftool (e.g., meld) to compare the two and move
        the changes into index.html.
    2.  `meld webdemo/collation/index.html ../available-collators.txt`
4.  See for example the changes for
    [ICU-11355](https://unicode-org.atlassian.net/browse/ICU-11355)

For details see the comments at the start of the build.sh file.

---

## Repository Branch and Tags

⚠ Careful! The following examples contain specific version, revision and ticket
numbers. Adjust them for the current release! Easiest: Paste into an editor,
fix, then paste into the terminal.

### Creating Maintenance Branch.

Sanity check: Update to the latest repository revision. (Main branch if you
copy from main, maintenance branch if you copy from there.)

```sh
git checkout main 
git pull upstream main 
git log -n1 
commit bcd0... (HEAD -> main, upstream/main, ...)
```

Ensure that your local branch is in sync with the upstream branch. Make sure you
are checking the upstream remote, and not your fork!

Build & test ICU4C & ICU4J on your machine.

Create the maintenance branch from the current known good main ref.

```sh
git checkout -b maint/maint-63 
git push -u upstream maint/maint-63
```

#### Tagging

Use the GitHub GUI to create both the "release" and the "tag" at the same time:

<https://github.com/unicode-org/icu/releases/new>

1. Fill in the tag name, such as `release-63-rc` or `release-63-1`, and make the
target the "maint/maint-xx" branch (such as `maint/maint-63`).
1. Set the title to `ICU 63 RC` or `ICU 63.1`.
1. Fill in the description using the
text from the announcement email. (You can also look at previous releases and
possibly re-use some of the generic text, such as links to the API docs, etc.)
1. Your screen should look like this:
    ![image](maint-63-rc-screenshot.png)
1. Check the box that says "Set as a pre-release".
1. Click the "Publish Release" button to make the tag.
1. Additional step only for the GA release:
After completing the step above to mark the release as "pre-release",
the second step is to wait for the day of the official announcement of the release,
and then to edit the Github release entry's settings.
    1.  The first Github release settings change should uncheck the "Set as a pre-release" checkbox,
which has the effect of converting the release into  a regular release.
    1. The next settings change should check the box that says "Set as the latest release".

Note: The "latest" tag is no longer updated. It was agreed by the ICU-TC to be
deleted around the 64.2 time-frame, as it doesn't work well with with Git. (You
need to force-push the new tag, and if somebody has already cloned the
repository, they might have something different for the "latest" tag).
A possible future alternative might be a sym-link folder, or HTTP redirect that
points to the latest release.

We no longer need to add the note about Git LFS files, as GitHub now includes
them in the auto-generated .zip downloads.

#### Maintenance release

Create the maintenance release tag from the maintenance branch, as above.

Update the "latest" tag.

### ~~ICU 58 and earlier~~

~~Tag related svn files, for **icu**, **icu4j** and (for final releases)
**tools** file trees. We tag the tools tree so that we can reproduce the Unicode
tools that were used for the Unicode data files in this release.~~

~~For a Release Candidate, just tag, don't branch, and only tag icu & icu4j.~~

~~For the final release, branch then tag. Copy the trunk to maint/maint-4-8 and
copy that to tags/release-4-8. Specify the source revision explicitly via -r so
that you don't inadvertently pick up an unexpected changeset. Make sure that the
trunk at the source revision is good.~~

~~We do not tag the data & icu-demos trees. Steven Loomis writes on 2011-may-23:~~

> ~~My thought had been (in the CVS days) to take a 'snapshot' of these items.
> However, in SVN all you need is a date or a revision number (such as
> r30140).~~

> ~~So, probably, we don't need to tag these two (idu-demos or data).~~

> ~~Tools are more important because those tools are actually used in the
> release.~~

### Create ICU download page

Create the download page before the first milestone, if we have one, or before
the release candidate.

Since ICU 76, new download pages are in Markdown on GitHub, at docs/download/ .

In your ICU workspace, copy the download page for the last release.
Adjust the navbar data at the top: Title, and nav_order one fewer than last time.

Adjust the new page as needed: Adjust the title to "ICU 77" (with the right version number...),
remove contents specific to the previous release, update all version numbers, update all links.

Put a big, **bold+italics** warning at the top like "This version has not been
released yet. Use it for testing but do not use it in production!"

Compare with the one-year-ago release page and adjust for whether we have a major release,
a new Unicode version, etc.

Add new contents for the upcoming release: Grab some text from the sibling
Unicode and CLDR release notes, look at the proposal status doc for this
release, make a pass through the api/enhancement tickets fixed in this release
or under reviewing/reviewfeedback.

Look at the download pages of the last two releases for templates for things
like a Migration Issues section etc.

Ask everyone on the team to add stuff & details.

Once the page has been created and merged, consider editing online on GitHub.

### Maintenance release

For a maintenance release, look at the ICU 60 page which includes 60.2.

---

## Milestone on the main download page

We had the following HTML on the main download page for ICU 4.8M1 = 4.7.1:

```html
<h3 style="background-color:rgb(102, 102, 102);color:white;margin-bottom:0pt;margin-top:12pt;padding-left:0.75em;font-size:1em;font-family:Arial,Helvetica,sans-serif">Development Milestones</h3>
<table border="0"><p style="font-size:10pt;font-family:Arial,Helvetica,sans-serif">Development milestone versions of ICU can be downloaded below. A development milestone is a stable snapshot build for next ICU major version.  These binaries and source code are provided for evaluation purpose and should be not be used in production environments.  New APIs or features in a milestone release might be changed or removed without notice.&nbsp;</p>
<tbody>
<tr>
<td style="width:105px;height:16px">&nbsp;<b>Release</b></td>
<td style="width:792px;height:16px">&nbsp;<b>Major Changes<br>
</b></td>
</tr>
<tr>
<td style="width:105px;height:29px">&nbsp;<a href="https://sites.google.com/site/icusite/download/471">4.8M1 (4.7.1)</a><br>
</td>
<td style="width:792px;height:29px">&nbsp;CLDR 1.9.1+, Parent locale override, Dictionary type trie, Alphabetic index (C), Compound text encoding (C), JDK7 Locale conversion (J)<br>
</td>
</tr>
</tbody>
</table>
</span><br>
```

---

## Upload Release Source / Binaries

Download Directories are located at, for example,
`icu-project.org:/home/htdocs/ex/files/icu4c/4.4.2`
corresponding to <http://download.icu-project.org/ex/files/icu4c/4.4.2/>
Look at previous releases for an example.

### Java Source/Bin

**Post 76.1 see [Publish - Version 76.1](release.md)**

Follow instructions here: [Building ICU4J Release Files](release.md)

### C source/binary:

**Post 76.1 see [Publish - Version 76.1](release.md)**

<span style="background:yellow">***WORK IN PROGRESS***</span>

#### Source and Linux Binaries:

**Post 76.1 see [Publish - Version 76.1](release.md)**

Important: this step works with Unix make + docker.

First, install *docker* and *docker-compose. D*o not proceed until *docker run
hello-world* works!

```sh
$ git clone https://github.com/unicode-org/icu-docker.git
$ cd icu-docker/src
$ git clone --branch release-64-rc --depth 1 https://github.com/unicode-org/icu.git
$ cd icu
$ git lfs install --local
$ git lfs fetch
$ git lfs checkout
$ cd ../..
$ less [README.md](https://github.com/unicode-org/icu-docker/blob/main/README.md)  # Follow these instructions.
```

*   Source and binaries are created in ./dist/.
*   The names [don't match what's needed on
    output](https://github.com/unicode-org/icu-docker/issues/1) so be sure to
    rename.

**Note:** If you only want to make a source tarball (.tgz/.zip), then you can
run \`make dist\`.

*   This will produce a source tarball and will include a pre-compiled .dat file
    under icu4c/source/data/in/.
*   Note: This tarball will also omit all of the data sub-directories containing
    locale data.
*   Note that the source is taken from the git repository itself, and not your
    local checkout. (Thus it will exclude any local uncommitted changes).

#### Windows Binary:

**Post 76.1 see [Publish - Version 76.1](release.md)** \
That new flow overlaps with _"Using the output from the build bots"_ below.

*   Manual process:
    *   Build with MSVC x64 Release. (See the ICU
        [readme.html](https://github.com/unicode-org/icu/main/blob/icu4c/readme.html)
        file for details).
    *   Open a command prompt.
        ```
        cd C:\icu\icu4c\ (or wherever you have ICU located).
        powershell
        Set-ExecutionPolicy -Scope Process Unrestricted
        .\packaging\distrelease.ps1 -arch x64
        ```
        This will produce the file "source\dist\icu-windows.zip", which will
        need to be renamed before uploading.
        *   For example, the binaries for ICU4C v61.1 generated with VS2017 were
            named "icu4c-61_1-Win64-MSVC2017.zip".
        *   Note: As of ICU 68, the pre-built binaries use MSVC2019 instead of
            MSVC2017.
*   Using the output from the build bots:
    *   Navigate to the GitHub page for the commits on the
        `maint/maint-<version>` branch.
        *   Ex: https://github.com/unicode-org/icu/commits/maint/maint-64
    *   Click on the green check mark (✔) on the most recent/last commit. (It
        might be a red X if the builds failed, hopefully not).
        *   This will open up a pop-up with links to various CI builds.
    *   Click on one of the various links that says "Details" for any of the GHA
        builds and click on "Summary".
        *   This will open up the GitHub overview of the build status.<br>
            ![image](gha-ci-summary.png)<br>
    *   Scroll down at the bottom to find the sub-section "Artifacts". It should show you list of zips you can download<br>
        ![image](gha-ci-artifacts.png)<br>
    *   Download the x64, x86 and ARM zip files.
    *   For each architecture:
        *   Extract the Zip file. (It will have a name like
            "icu4c.Win64.run_#104.zip").
        *   Navigate into the folder with the same name.
        *   Check and verify the names of the zip file are appropriate:
            *   Ex: The x64 zip for version 76.1 should be named
                "icu4c-76_1-Win64-MSVC2022.zip"
            *   Ex: The x86 zip for version 76.1 should be named
                "icu4c-76_1-Win32-MSVC2022.zip"
        *   Note: For RC releases the name looked like this:
            "icu4c-76rc-Win64-MSVC2022"
*   ~~AIX Bin:~~ (AIX is broken and ignored for now.)
    *   ~~login to gcc119.fsffrance.org and copy the ICU4C source archive
        created above to there.~~
    *   ~~$ gzip -dc icu4c-XXX-src.tgz | tar xf -~~
    *   ~~$ cd icu~~
    *   ~~$ PATH=/opt/IBM/xlC/13.1.3/bin:$PATH source/runConfigureICU AIX~~
    *   ~~(The above command line doesn't actually work, see [ICU Ticket
        ICU-13639](https://unicode-org.atlassian.net/browse/ICU-13639) for a
        workaround.)~~
    *   ~~$ gmake DESTDIR=/tmp/icu releaseDist~~
    *   ~~That last step will create a directory in **/tmp/icu** - zip that up
        to make the release.~~
    *   ~~In case /tmp happens to be full, see the [mailing list
        archive](https://sourceforge.net/p/icu/mailman/message/36275940/) for
        advice.~~

#### Output of icuexportdata:

**Post 76.1 see [Publish - Version 76.1](release.md)**

This step publishes pre-processed Unicode property data, which may be ingested by downstream clients such as ICU4X.

*   Using the output from the build bots:
    *   Navigate to the GHA Workflow `icu4c-icuexportdata` and download the artifact (`icuexportdata_output`) from summary page
    *   Unzip the file
    *   Rename the `icuexportdata_tag-goes-here.zip` file to the correct tag (replacing slashes with dashes)

### Signing archives and creating checksums:

**Post 76.1 see [Publish - Version 76.1](release.md)**

#### Step 0. PGP keys:

Use your own personal PGP key. Make sure that at least one other member of the
ICU-TC has signed your key (so that there's an established chain-of-trust
through which your key can be verified). Make sure that your signed public key
is included in the `KEYS` file in the root of the ICU repository, so that users
of ICU can easily find it there (and won't have to search random keyservers for
it), see instructions in that file on how to update it.

#### Step 1. PGP files:

Sign all archives created above with your own personal PGP key. This creates a
file with .asc as the suffix.

```sh
$ gpg --armor --detach-sign icu4c-xxx-xxx.zip
# To verify
$ gpg --verify icu4c-xxx-xxx.zip.asc
```

#### Step 2. MD5 files:

Use md5sum or [cfv](http://cfv.sf.net) to create [md5](https://en.wikipedia.org/wiki/MD5) hash sums for three groups of files:

*   icu4j (all files),
*   icu4c (source),
*   icu4c (binaries).

Using md5sum to create and verify the checksum files:

<pre><code><b><b>md5sum source1 source2 ... sourceN &gt; icu4c_sources.md5</b></b> # To verifymd5sum -c icu4c_sources.md5 
</code></pre>

Alternatively, use cfv to create and verify md5 files:

```sh
cfv -t md5 -C -f icu-……-src.md5 somefile.zip somefile.tgz …
# To verify 
cfv -f icu-……-src.md5
```

#### Step 3. SHASUM512.txt

Create an additional hash sum file SHASUM512.txt file with:

```sh
shasum -a 512 *.zip *.tgz | tee SHASUM512.txt
```

This file should also be GPG signed. Check the .asc with \`gpg verify\`.

### Update the Download Page Gadgets

Update the gadgets on the download page to point at the new URL for the
binaries.

1.  Edit the download page.
2.  Click on the Gadget area.
3.  Click on the "gear" icon.
4.  Update the URL field with the new URL.
    1.  For example: The ICU4C 63.1 Binaries URL was:
        <http://apps.icu-project.org/icu-jsp/downloadSection.jsp?ver=63.1&base=c&svn=release-63-1>

#### Check the ICU public site for the new release

Make sure that, aside from download pages, homepages, news items, feature lists
and feature comparisons, etc. are updated. Upload the new API references. Update
the User Guide.

#### Update the Trac release number list for ICU4C and ICU4J. <<?? STILL VALID ??>>

Update the ICU release number list by going to "Admin>Versions" in Trac, and add
the new ICU version.

#### Post-release cleanup

*   Cleanup the milestone in the ICU Trac. Move left over items to future
    milestones. Close the milestone.
*   Look for TODO comments in the source code and file new tickets as required.
*   Delete and retag
    [latest](http://source.icu-project.org/repos/icu/tags/latest/) (**ONLY**
    after GA release, including maintenance!) << IS THIS STILL VALID WITH GIT?
    >>

---

## Update online demos

These are the online demos/tools that need to be updated to the latest version.

* Be sure to verify that the deployed version is publicly available.

Note that updating ICU4C demos online requires Gcloud access.

### ICU4C demos
* [Run ICU4C demos](https://icu4c-demos.unicode.org/icu-bin/idnbrowser)

* [Demo described here](https://github.com/unicode-org/icu-demos/blob/main/README.md)

* [Building and deploying from GCloud](https://github.com/unicode-org/icu-demos//blob/main/README.md)

### ICU4J demos

* [Run ICU4J online demos](https://icu4j-demos.unicode.org/icu4jweb/)

* [Information on the Java demos and samples](https://icu.unicode.org/home/icu4j-demos)

* [Instructions for building and deploying updates](https://github.com/unicode-org/icu-demos/blob/main/icu4jweb/README.md)


### Online information update

Collation and [comparison](../../../../charts/comparison/index.md) charts need
to be updated. See [charts/Performance & Size](../../../../charts/index.md).

### Old sensitive tickets

Unset the "sensitive" flag on old tickets. For example, on tickets that were
fixed two or more releases ago.

[Sample ticket query for ICU 65, for tickets fixed in 63 or
earlier](https://unicode-org.atlassian.net/issues/?jql=project%20%3D%20ICU%20AND%20Level%3DSensitive%20AND%20fixVersion%20not%20in%20(65.1%2C%2064.2%2C%2064.1)%20AND%20status%3DDone).
Adjust the fixVersion selection as appropriate. Check the list in the ICU
meeting.

Check duplicates and fixedbyotherticket! Keep the "sensitive" flag on tickets
that were closed as duplicates of other tickets that are not yet fixed or have
been fixed only very recently.

For removing the flag:

*   Enter bulk edit. Select all query results.
*   Uncheck duplicates of unfixed or too-recent tickets.
*   Edit fields:
    *   Security Level = None
    *   Add label "was_sensitive"
    *   No notification emails
*   Confirm bulk edit.

---

## Punt tickets

Double-check that tickets with commits in this release are closed/fixed. Close
as needed. (Hopefully none misticketed at this point...)

Then punt remaining tickets marked for this release:

1.  In Jira, search for all tickets with Project=ICU, Fix Version=<this
    release>, Status≠Done.
2.  Go to Bulk Edit ("..." menu in the upper right corner)
3.  Select all
4.  Edit fields:
    1.  Fix Version: replace all with "future"
    2.  Labels: add "punt<this release>" (e.g., "punt63")
    3.  No email notifications
5.  Confirm bulk edit
6.  Send a courtesy email to the team with a Jira query URL for the
    Label="punt<this release>" tickets.

After punting, please also update the "To-Do for Next ICU Release" widget on
Jira.

1.  Open <https://unicode-org.atlassian.net/issues/?filter=10007>
2.  Use the drop-down to change the fix version to the next ICU version
3.  Click "Save" next to the filter title

## Update readme

Update [ICU4C
readme.html](https://github.com/unicode-org/icu/main/icu4c/readme.html)
and [ICU4J
readme.html](https://github.com/unicode-org/icu/main/icu4j/readme.html)
before every milestone (GA / RC / Milestone-N). Make sure the following items
are up to date.

*   Release version
*   Last update date
*   Description - descriptions for GA, RC and Milesone-N are already included in
    the readme file.
    *   Comment/uncomment parts as appropriate.
    *   If the readme should remain the same between milestones, we can skip
        directly to the GA description. Otherwise, pick the right one for the
        release type.
    *   **Since ICU 67, we have skipped from GA to GA**, without marking &
        unmarking the readme specifically for the release candidate.
*   Build steps - make sure supported compiler versions are up to date
