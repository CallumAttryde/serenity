#pragma once

#include <AK/Retained.h>
#include <Kernel/Assertions.h>
#include <Kernel/PhysicalAddress.h>

class PhysicalPage {
    friend class MemoryManager;
    friend class PageDirectory;
    friend class VMObject;

public:
    PhysicalAddress paddr() const { return m_paddr; }

    void retain()
    {
        ASSERT(m_retain_count);
        ++m_retain_count;
    }

    void release()
    {
        ASSERT(m_retain_count);
        if (!--m_retain_count) {
            if (m_may_return_to_freelist)
                move(*this).return_to_freelist();
            delete this;
        }
    }

    static Retained<PhysicalPage> create(PhysicalAddress, bool supervisor, bool may_return_to_freelist = true);

    word retain_count() const { return m_retain_count; }

private:
    PhysicalPage(PhysicalAddress paddr, bool supervisor, bool may_return_to_freelist = true);
    ~PhysicalPage() {}

    void return_to_freelist() &&;

    word m_retain_count { 1 };
    bool m_may_return_to_freelist { true };
    bool m_supervisor { false };
    PhysicalAddress m_paddr;
};
