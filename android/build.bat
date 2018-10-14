

SET PREV_PATH=%CD%
cd /d %0\..

rmdir "bin" /S /Q
rmdir "gen" /S /Q
rmdir "assets" /S /Q
mkdir "bin"
mkdir "gen"
mkdir "assets"

SET ANDROID_REV=android-21
SET ANDROID_BUILD_TOOLS_VERS=21.1.2
SET ANDROID_AAPT_ADD="%ANDROID-SDK%\build-tools\%ANDROID_BUILD_TOOLS_VERS%\aapt.exe" add -v
SET ANDROID_AAPT_PACK="%ANDROID-SDK%\build-tools\%ANDROID_BUILD_TOOLS_VERS%\aapt.exe" package -v -f -I "%ANDROID-SDK%\platforms\%ANDROID_REV%\android.jar"
SET ANDROID_DX="%ANDROID-SDK%\build-tools\%ANDROID_BUILD_TOOLS_VERS%\dx.bat" --dex --verbose
SET ANDROID_ZIPALIGN="%ANDROID-SDK%\build-tools\%ANDROID_BUILD_TOOLS_VERS%\zipalign.exe"
SET JAVAC="%JAVA_HOME%\bin\javac.exe" -classpath "%ANDROID-SDK%\platforms\%ANDROID_REV%\android.jar";"D:\repos\raymark\android\libs\classes.jar";classes

SET APP_NAME=OccDemo
SET JAVAC_BUILD=%JAVAC% -source 1.7 -target 1.7 -bootclasspath "%JAVA_HOME%\jre\lib\rt.jar" -sourcepath "src;gen;libs" -d "bin"

xcopy "%CD%\..\assets" "%CD%\assets\" /E /C /I /Y || exit \b

call %JAVAC_BUILD% src\com\serg\occdemo\*.java || exit \b

call %ANDROID_AAPT_PACK% -M "AndroidManifest.xml" -A "assets" -S "res" -m -J "gen" -F "%CD%\bin\resources.ap_" || exit \b

call %ANDROID_DX% --output="%CD%\bin\classes.dex" %CD%\bin || exit \b

copy "%CD%\bin\resources.ap_" "%CD%\bin\%APP_NAME%.ap_" || exit \b

cd bin
call %ANDROID_AAPT_ADD% "%APP_NAME%.ap_" "classes.dex" || exit \b
cd ..

REM call %ANDROID_AAPT_ADD% "%CD%\bin\%APP_NAME%.ap_" "lib/armeabi/libDemoApp.so" || exit \b
REM call %ANDROID_AAPT_ADD% "%CD%\bin\%APP_NAME%.ap_" "lib/armeabi-v7a/libDemoApp.so" || exit \b
call %ANDROID_AAPT_ADD% "%CD%\bin\%APP_NAME%.ap_" "lib/arm64-v8a/libDemoApp.so" || exit \b
call %ANDROID_AAPT_ADD% "%CD%\bin\%APP_NAME%.ap_" "lib/x86/libDemoApp.so" || exit \b
REM call %ANDROID_AAPT_ADD% "%CD%\bin\%APP_NAME%.ap_" "lib/x86_64/libDemoApp.so" || exit \b

call "%JAVA_HOME%\bin\jarsigner" -keystore "%CD%\keystore\my-release-key.keystore" -storepass "fxF5vbttHc" -keypass "fxF5vbttHc" -tsa "http://timestamp.comodoca.com/rfc3161" -sigalg SHA1withRSA -digestalg SHA1 -signedjar "%CD%\bin\%APP_NAME%.ap_" "%CD%\bin\%APP_NAME%.ap_" "serg" || exit \b

call "%JAVA_HOME%\bin\jarsigner" -keystore "%CD%\keystore\my-release-key.keystore" -verify "%CD%\bin\%APP_NAME%.ap_" || exit \b

call %ANDROID_ZIPALIGN% -f -v 4 "%CD%\bin\%APP_NAME%.ap_" "%CD%\bin\%APP_NAME%.apk"

:EXIT
cd "%PREV_PATH%"
ENDLOCAL
exit /b %ERRORLEVEL%