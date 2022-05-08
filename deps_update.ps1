function Get-Projects
{
    $patt = "(GitClonedProjects)"
    $repos = Select-String -Path "deps/*/*" -Pattern $patt 
    $projects = New-Object Collections.Generic.List[string]($repos.Length)

    foreach($info in $repos)
    {
        $str = $info.ToString()
        $offset = $str.IndexOf($patt) + $patt.Length + 1
        $arr = $str.Substring($offset) -split "\\"
        $result =  $arr[0] + '\' + $arr[1]

        foreach($c in @(';', '<'))
        {
            $idx = $result.IndexOf($c)
            if($idx -ne -1)
            {
                $result = $result.Substring(0, $idx)
            }        
        }
        $projects.Add($result)
    }
    Write-Output $projects | select -Unique
}

function Get-ReposDir
{
    $prop = [xml](Get-Content "Props\Configuration.props")
    foreach($obj in $prop.Project.PropertyGroup.GitClonedProjects)
    {
        if($obj -ne $null -and $obj.Length -ne 0)
        {
            Write-Output $obj
        }
    }
}

$projects = Get-Projects
#echo $projects
$repos_dir = Get-ReposDir
#echo $repos_dir

foreach($tmp in $projects)
{
    $proj = $tmp.ToString()
    $clone = "https://github.com/"+$proj.Replace('\','/')
    $path = $repos_dir  +'\' + $proj      

    git.exe clone $clone $path
    git.exe pull $path

    #echo $clone
    #echo $path
}

#pause

