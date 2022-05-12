$GLOBAL_PATT = "GitClonedProjects"

function Get-RepoPath($str)
{
    $word1_end = $str.IndexOf('\')
    foreach($c in  @('\', '<', ';'))
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

function Make-Url($site, $str)
{
    "https://$site/" + $str.Replace('\', '/')
}

function Count-Substring($str, $substr)
{
   $count = 0
   $pos = 0
   for(;;)
   {
        $pos = $str.IndexOf($substr, $pos)
        if($pos -eq -1)
        {
            break
        }
        ++$count        
        $pos += $substr.Length
   }
   $count
}

#multiline comments unsupported!
function Inside-XmlComment($str)
{
    $open = Count-Substring $str "<!--"
    $close = Count-Substring $str "-->"
    $open -ne $close
}

function Get-Substring($str, $front, $back)
{
    $offset_start = $str.IndexOf($front) + $front.Length
    $offset_end = $str.LastIndexOf($back)
    $size = $offset_end - $offset_start

    $str.Substring($start_offset, $size)
}

function Get-Projects([string[]]$data)
{
    foreach($path in $data)
    {
        $patt = Add-Braces $GLOBAL_PATT
        $repos = Select-String -Path $path -Pattern $patt

        foreach($info in $repos)
        {
            $whole_str = $info.ToString()
            $str_begin = $whole_str.IndexOf($patt) + $patt.Length + 1
            $str_before = $whole_str.Substring(0, $str_begin)
            if(Inside-XmlComment $str_before)
            {
                continue
            }
            Get-RepoPath $whole_str.Substring($str_begin)
        }
    }
}

function Get-ReposDir
{
    $path = "Props/Configuration.props"
    $patt_front = Add-XmlFront $GLOBAL_PATT
    $patt_back = Add-XmlBack $GLOBAL_PATT
    $data = Select-String -Path $path -Pattern $patt_front

    Get-Substring $data.ToString() $patt_front $patt_back
}

$projects = (Get-Projects "deps/*/*", "impl/*.props") | Get-Unique
#echo $projects
$repos_dir = Get-ReposDir
#echo $repos_dir

$msg = 0

foreach($tmp in $projects)
{
    $proj = $tmp.ToString()
    $clone = Make-Url "github.com" $proj
    $path = Join-Path $repos_dir $proj      
    
    if(-not (Test-Path $path))
    {
        git.exe clone $clone $path
        $msg++
    }
    
    $result = git.exe -C $path pull
    if($msg -eq 0 -and $result -ne "Already up to date.")
    {
        $msg++
    }
    echo "Pulling '$proj'..."
    echo $result      

    #echo $clone
    #echo $path
}

if($msg -ne 0)
{
    pause
}

