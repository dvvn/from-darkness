$GLOBAL_PATT = "GitClonedProjects"
$PATH_DELIMNER = '\'

function Get-RepoPath($str)
{
    $word1_end = $str.IndexOf($PATH_DELIMNER)
    foreach($c in  @($PATH_DELIMNER, '<', ';'))
    {
        $word2_end = $str.IndexOf($c, $word1_end + 1)
        if($word2_end -ne -1)
        {
            $str.Substring(0, $word2_end)
            break
        }
    }    
}

function Add-Braces($obj)
{
    "($obj)"
}

function Add-XmlFront($obj)
{
    "<$obj>"
}

function Add-XmlBack($obj)
{
    "</$obj>"
}

function Make-Path($front, $back)
{
    $front + $PATH_DELIMNER + $back
}

function Make-Url([string]$site, [string]$str)
{
    "https://$site/" + $str.Replace($PATH_DELIMNER, '/')
}

function Get-Projects
{
    $path = "deps/*/*"
    $patt = Add-Braces $GLOBAL_PATT
    $repos = Select-String -Path $path -Pattern $patt    

    foreach($info in $repos)
    {
        $whole_str = $info.ToString()
        $str_begin = $whole_str.IndexOf($patt) + $patt.Length + 1
        Get-RepoPath $whole_str.Substring($str_begin)
    }
}

function Get-ReposDir
{
    $path = "Props/Configuration.props"
    $patt_front = Add-XmlFront $GLOBAL_PATT
    $patt_back = Add-XmlBack $GLOBAL_PATT
    $data = Select-String -Path $path -Pattern $patt_front

    $str = $data.ToString()
    $start_offset = $str.IndexOf($patt_front) + $patt_front.Length
    $end_offset = $str.LastIndexOf($patt_back)
    $size = $end_offset - $start_offset

    $str.Substring($start_offset, $size)
}

$projects = Get-Projects | Get-Unique
#echo $projects
$repos_dir = Get-ReposDir
#echo $repos_dir

$errors = 0

foreach($tmp in $projects)
{
    $proj = $tmp.ToString()
    $clone = Make-Url "github.com" $proj
    $path = Make-Path $repos_dir $proj      
    
    if(Test-Path $path)
    {
        $result = git.exe -C $path pull
        if($errors -eq 0 -and $result -ne "Already up to date.")
        {
            $errors++
        }
        echo "Pulling '$proj'"
        echo $result
    }
    else
    {
        git.exe clone $clone $path
        $errors++
    }   

    #echo $clone
    #echo $path
}

if($errors -ne 0)
{
    pause
}

