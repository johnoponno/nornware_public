#pragma once

template <typename type_t>
struct traversable_t
{
	static void delete_all()
	{
		while (first)
			delete first;
	}

	virtual ~traversable_t()
	{
		//unlink
		if (this == first)
		{
			first = next;

			if (this == last)
			{
				assert(!first);
				last = nullptr;
			}
		}
		else
		{
			type_t* previous = first;
			type_t* current = previous->traversable_t<type_t>::next;

			while (current != this)
			{
				previous = current;
				current = current->traversable_t<type_t>::next;
			}

			previous->traversable_t<type_t>::next = traversable_t<type_t>::next;

			if (this == last)
				last = previous;
		}

		//decrease instance count
		assert(count);
		--count;

		//debug
		//log::leak(ourName, ourNumInstances);
	}

	type_t* next_wrap() const
	{
		if (next)
			return next;
		return traversable_t<type_t>::first;
	}

	type_t* previous_wrap() const
	{
		type_t* i = first;

		//nothing in list
		if (!i)
			return nullptr;

		//this is the first instance, so return the last
		if (this == i)
		{
			while (i->next)
				i = i->next;
			return i;
		}

		//search for the one before this
		while (this != i->next)
			i = i->next;

		return i;
	}

	static type_t* find(const type_t* anInstance)
	{
		type_t* current = first;

		while (current)
		{
			if (current == anInstance)
				return current;
			current = current->traversable_t<type_t>::next;
		}

		return nullptr;
	}

	static type_t* first;
	static uint32_t count;
	static uint32_t high;
	type_t* next;

protected:

	explicit traversable_t()
		:next(nullptr)
	{
		//link
		if (last)
		{
			assert(first);
			last->traversable_t<type_t>::next = (type_t*)this;
			last = (type_t*)this;
		}
		else
		{
			assert(!first);
			first = last = (type_t*)this;
		}

		//increase instance count
		++count;
		high = __max(high, count);
	}

private:

	//members
	static type_t* last;
};

#define TRAVERSABLE(x) x* traversable_t<x>::first = nullptr; x* traversable_t<x>::last = nullptr; uint32_t traversable_t<x>::count = 0; uint32_t traversable_t<x>::high = 0;
