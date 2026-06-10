$md5 = [System.Security.Cryptography.MD5]::Create()

function GetHash($str) {
    $bytes = [System.Text.Encoding]::ASCII.GetBytes($str)
    $hash = $md5.ComputeHash($bytes)
    return [BitConverter]::ToString($hash).Replace('-', '')
}

$lines = Get-Content "e:\habsolution\assets\HeaderLibraries.txt"
foreach ($line in $lines) {
    $parts = $line.Split(':')
    if ($parts.Length -ge 2) {
        $chunkName = $parts[1..($parts.Length-1)] -join ':'
        if ($chunkName -match "^(.*\])\.pc_headerlib$") {
            $baseName = $matches[1]
            $strToHash = $baseName.ToLower() + "."
            $hash = GetHash $strToHash
            Write-Host "$hash : $baseName"
        }
    }
}
