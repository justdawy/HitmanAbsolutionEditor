$md5 = [System.Security.Cryptography.MD5]::Create()

function GetHash($str) {
    $bytes = [System.Text.Encoding]::ASCII.GetBytes($str)
    $hash = $md5.ComputeHash($bytes)
    return [BitConverter]::ToString($hash).Replace('-', '')
}

$chunks = @('menu', 'l01', 'l01b', 'l02b', 'l02c', 'l02d', 'l03', 'l04b', 'l04c', 'l04d', 'l04e', 'l05a', 'l05b', 'l05c', 'l06a', 'l06d', 'l07a', 'l08a', 'l08b', 'l09', 'l10', '0060', '7221')

foreach ($chunk in $chunks) {
    $str = "[[assembly:/common/pc.layoutconfig].pc_layoutdef]($chunk)."
    $hash = GetHash $str
    Write-Host "$chunk : $hash"
}
