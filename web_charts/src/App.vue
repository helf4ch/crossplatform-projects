<script setup>
import { ref, onMounted } from 'vue';
import axios from 'axios';

const records = ref([]);
const hours = [1, 3, 6, 12, 24];
const selectedHours = ref(1);

const fetchData = async () => {
  try {
    const response = await axios.get('http://localhost:8080/get',  {
      params: { hours: selectedHours.value },
    });
    records.value = response.data.reverse();
    console.log(response.data);
  } catch (error) {
    console.error(error);
  }
}

const formatUnixTime = (unixtime) => {
  const date = new Date(unixtime * 1000);
  return date.toLocaleString('ru-RU');
}

onMounted(() => {
  fetchData();
})
</script>

<template>
  <h1>{{ message }}</h1>
  <div id="app">
    <h1>Погодные данные</h1>
    <div>
      <label for="hours">Период данных (в часах): </label>
      <select id="hours" v-model="selectedHours" @change="fetchData">
        <option v-for="hour in hours" :key="hour" :value="hour">
          {{ hour }}
        </option>
      </select>
    </div>
    <button @click="fetchData">Обновить данные</button>
    <table v-if="records.length" border="1">
      <thead>
        <tr>
          <th>Время</th>
          <th>Температура</th>
          <th>Давление</th>
          <th>Влажность</th>
          <th>Скорость ветра</th>
          <th>По ощущению</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="record in records" :key="record.unixtime">
          <td>{{ formatUnixTime(record.unixtime) }}</td>
          <td>{{ record.temperature.toFixed(1) }}</td>
          <td>{{ record.pressure }}</td>
          <td>{{ record.humidity }}</td>
          <td>{{ record.wind_speed.toFixed(1) }}</td>
          <td>{{ record.feels_like.toFixed(1) }}</td>
        </tr>
      </tbody>
    </table>
    <p v-else>No data. Press the button.</p>
  </div>
</template>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  text-align: center;
  color: #2c3e50;
  margin: 20px;
}

table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 20px;
}

th, td {
  padding: 10px;
  text-align: center;
}

th {
  background-color: #f4f4f4;
}

tr:nth-child(even) {
  background-color: #f9f9f9;
}

select {
  margin: 10px;
  padding: 5px;
  font-size: 16px;
}
</style>
