$raw = Get-Content "$PSScriptRoot\data.json" -Raw | ConvertFrom-Json
$records = @()
$id = 1
foreach ($row in $raw) {
    if ($row.Count -ge 4 -and $row[0] -ne 'Date') {
        $rem = if ($row.Count -ge 5) { $row[4] } else { '' }
        $records += [PSCustomObject]@{
            id = $id
            date = $row[0].Trim()
            place = $row[1].Trim()
            item = $row[2].Trim()
            desc = $row[3].Trim()
            remark = $rem.Trim()
        }
        $id++
    }
}
$embeddedJson = $records | ConvertTo-Json -Compress
$html = Get-Content "$PSScriptRoot\index.html" -Raw
$target = 'let database = [];'
$replacement = "let EMBEDDED_RECORDS = $embeddedJson;`r`n    let database = [...EMBEDDED_RECORDS];"
$newHtml = $html.Replace($target, $replacement)
[System.IO.File]::WriteAllText("$PSScriptRoot\index.html", $newHtml, [System.Text.Encoding]::UTF8)
Write-Output "Embedded $($records.Count) records into index.html successfully."
