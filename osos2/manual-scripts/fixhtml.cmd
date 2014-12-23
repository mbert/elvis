@echo off
if not exist %1 goto end
echo fixing %1...
ed -s %1 <ctags.man.ed     2>&1 | grep -v "^\?"
ed -s %1 <elvis.man.ed     2>&1 | grep -v "^\?"
ed -s %1 <elvis.syn.ed     2>&1 | grep -v "^\?"
ed -s %1 <elvis.ini.ed     2>&1 | grep -v "^\?"
ed -s %1 <elvis.ali.ed     2>&1 | grep -v "^\?"
ed -s %1 <elvistrs.msg.ed  2>&1 | grep -v "^\?"
ed -s %1 <fmt.man.ed       2>&1 | grep -v "^\?"
ed -s %1 <ref.man.ed       2>&1 | grep -v "^\?"
:end
