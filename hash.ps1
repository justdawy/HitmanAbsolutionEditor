$md5 = [System.Security.Cryptography.MD5]::Create()

function GetHash($str) {
    $bytes = [System.Text.Encoding]::ASCII.GetBytes($str)
    $hash = $md5.ComputeHash($bytes)
    return [BitConverter]::ToString($hash).Replace('-', '')
}

Write-Host "7221:" (GetHash '[[assembly:/common/pc.layoutconfig].pc_layoutdef](7221).')
Write-Host "0060:" (GetHash '[[assembly:/common/pc.layoutconfig].pc_layoutdef](0060).')
Write-Host "fonts_ru_swf:" (GetHash '[assembly:/ui/common/fonts_ru.swf].')
Write-Host "fonts_ru_multi:" (GetHash '[assembly:/ui/common/fonts_ru.swf](multilanguage).')
