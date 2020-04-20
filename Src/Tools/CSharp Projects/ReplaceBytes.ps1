param([string] $cpppath, [string]$dllpath)

[regex]$firstRegex = "unsigned char assemblyDLL\[\] \= \{(0x..(,)?)+"
[regex]$secondRegex = "(0x..(,)?)+( )?}"

$FirstLine = (gci $cpppath | select-string "unsigned char assemblyDLL").LineNumber

#Get Last Line Works
$LastLine = (gci $cpppath | select-string $secondRegex).LineNumber

#Delete Old DLL Content
$contents = (Get-Content $cpppath | Where-Object {
    -not ($_.ReadCount -ge $FirstLine -and $_.ReadCount -le $LastLine)
})

#Replace Contents
$Bytes = Get-Content $dllpath -Encoding Byte
$HexString = [System.Text.StringBuilder]::new($Bytes.Length * 4)
ForEach($byte in $Bytes) { $HexString.AppendFormat("0x{0:x2},", $byte) | Out-Null }

$HexString = $HexString.ToString().TrimEnd(',')

$contents | Foreach-Object {
        $_ # send the current line to output
        if ($_ -match "//AssemblyDLL Goes Here") 
        {
            #Add Lines after the selected pattern 
            "`t`tunsigned char assemblyDLL[] = { $HexString };"
        }
    } | Set-Content $cpppath