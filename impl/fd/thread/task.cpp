module;

module fd.task;

using namespace fd;

void basic_finished_task::start()
{
}

void basic_finished_task::wait()
{
}

finished_task::finished_task()
    : task(basic_finished_task())
{
}
